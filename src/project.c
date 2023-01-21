#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>

#include "util.h"
#include "project.h"
#include "filesystem.h"



/*
 * Function initializes a new project
 *
 * some things it does:
 *  - create project directory
 *  - create sub-directories
 *  - initialize git repository
 *  - Adds src and tests sub-directories to git
 *  - Print success message with path to repo
 */
int create_project(char *project_name, char *error)
{
    // a temporary variable for command results
    int result = 0;

    // Before using this file for commands and file-system
    // operations we need to make sure it doesn't have any
    // "unsafe" characters
    int path_valid = check_valid_string(project_name, SAFE_CHARS_FILENAME);
    if (path_valid != 0)
        return path_valid;


    // Make sure no file or directory exists with this name.
    if (path_exists(project_name))
    {
        snprintf(error, MAX_ERROR, "A file or directory with this name already exists. Cannot create project\n");
        return EEXIST;
    }


    // Create the new project directory
    result = 0;
    result = safe_mkdir(project_name);
    if (result != 0)
    {
        snprintf(error, MAX_ERROR, "An error occurred while creating project directory");
        return result;
    }

    // create sub-directories for the project
    result = 0;
    result = create_default_sub_dirs(project_name);
    if (result != 0)
        return result;


    // Initialize the git repository
    // We can safely add `project_name` to this system
    // command because we ran `check_valid_string` on
    // it before.
    char command[MAX_OBJECT_NAME + 41] = "";
    snprintf(command, MAX_OBJECT_NAME + 40, "cd %s && git init --quiet > /dev/null", project_name);
    result = 0;
    result = system(command);
    if (result != 0)
    {
        snprintf(error, MAX_ERROR, "An error occurred while initializing the git repository\n");
        return result;
    }

    // Add the `src` and `tests` directories to git.
    // Not adding the others as I don't think those
    // directories need to be saved to remote repositories.
    char git_command[MAX_OBJECT_NAME + 31];
    snprintf(git_command, MAX_OBJECT_NAME + 30, "cd %s && git add src tests", project_name);
    result = 0;
    result = system(git_command);
    if (result != 0)
    {
        snprintf(error, MAX_ERROR, "An error occurred while adding 'src' and 'tests' to the git repository.\n");
        return result;
    }

    // Print success message and path to repository.
    char current_path[PATH_MAX];
    getcwd(current_path, PATH_MAX);
    printf("Initialized empty repository in %s/%s/.git\n", current_path, project_name);
    return 0;
}


/*
 * Function adds a feature branch to the project
 *
 * Technically there is nothing "feature" related
 * about the function. It creates a new directory
 * by the name of the `feature_name` argument,
 * and creates the same sub-directories as
 * create_project does.
 */
int add_feature(char *feature_name, char *project_path, char *error)
{
    // A variable to hold project_path/feature_name
    // The buffer has to be large enough to hold the
    // maximum linux path length plus the maximum
    // length a feature_name can be. 
    char fullpath[PATH_MAX + MAX_OBJECT_NAME + 1 + 1] = "";
    snprintf(fullpath, MAX_OBJECT_NAME + MAX_OBJECT_NAME + 1, "%s/%s", project_path, feature_name);

    // Temporary variable for holding the
    // results of functions.
    int result = 0;

    // Make sure the new feature name is safe
    // NOTE: it checks feature_name instead of
    // full_path because full_path has `/` 
    // characters in it which are banned by
    // `SAFE_CHARS_FILENAME`.
    result = 0;
    result = check_valid_string(feature_name, SAFE_CHARS_FILENAME);
    if (result != 0)
        return result;


    // Make sure the directory to create the
    // feature in actually exists.
    if (!path_exists(project_path))
    {
        snprintf(error, MAX_ERROR, "No such project: '%s'", project_path);
        return ENOENT;
    }

    // Make sure no file or directory by this name
    // already exists.
    if (path_exists(fullpath))
    {
        snprintf(error, MAX_ERROR, "A feature by that name already exists");
        return EEXIST;
    }


    // Create feature directory.
    result = 0;
    result = mkdir(fullpath, NEW_FILE_PERMISSIONS);
    if (result != 0)
    {
        snprintf(error, MAX_ERROR, "An error occurred while creating feature directory");
        return result;
    }


    // Create default sub-directories.
    result = 0;
    result = create_default_sub_dirs(fullpath);
    if (result != 0)
        return result;


    // Print success message.
    printf("New feature '%s' added at: %s/", feature_name, fullpath);
    return 0;
}



/*
 * Function creates a file called .pm_tag with the contents
 * of `tag_name`.
 */
int add_tag(char *tag_name, char *project_path, char *error)
{
    // NOTE: don't need to `check_valid_string` here
    // as to the best of my knowledge, nothing can be
    // injected with the fprintf command when writing
    // to file. (if handled correctly)


    // Make sure the actual directory to create the
    // file in actually exists.
    if (!path_exists(project_path))
    {
        snprintf(error, MAX_ERROR, "No such project: '%s'", project_path);
        return ENOENT;
    }

    // Create a full path to the `.pm_tag` file that
    // we need to create.
    char pm_tag_path[PATH_MAX + MAX_OBJECT_NAME + 8 + 1] = "";
    snprintf(pm_tag_path, PATH_MAX + MAX_OBJECT_NAME + 8, "%s/%s", project_path, "/.pm_tag");


    // Write contents to tag file.
    int return_code = safe_write_file(pm_tag_path, tag_name);
    if (return_code == EEXIST)
    {
        // Error if a file or directory already exists
        // by this name.
        //
        // NOTE: my program crashes when a tag exists,
        // I'm unsure if it should instead it should just
        // over-write the file.
        snprintf(error, MAX_ERROR, "A tag for this feature already exists");
        return return_code;
    }
    else if (return_code == EPERM)
    {
        // Error if the file could not be opened for writing.
        snprintf(error, MAX_ERROR, "Could not open file for writing");
        return return_code;
    }

    // Print success message.
    printf("Added tag '%s'", tag_name);
    return 0;
}



/*
 * Find a tag
 *
 * The response of this function is returned
 * in a `response` buffer. This also substitutes the
 * `error` buffer in most of the rest of the codebase.
 * (this should also be MAX_ERROR sized)
 */
int find_tag(char *tag_name, char *project_path, char *response)
{
    // Make sure the project exists
    if (!path_exists(project_path))
    {
        snprintf(response, MAX_ERROR, "No such project: '%s'", project_path);
        return ENOENT;
    }


    // Variable to hold the path to the tag.
    char tag_path[PATH_MAX+1] = "";

    // Try find the tag. Returns true or false
    if (find_file_by_contents(project_path, ".pm_tag", tag_name, tag_path))
        // print success message
        snprintf(response, PATH_MAX+MAX_OBJECT_NAME+10, "%s: %s\n", tag_name, tag_path);

    else
    {
        // could not find tag
        strncpy(response, "Could not find tag!", MAX_ERROR);
        return ENOENT;
    }

    return 0;
}


/*
 * Rename a feature.
 *
 * The function renames a file or directory.
 * Although it is called rename_feature, it
 * technically will happily rename anything
 * else as well.
 */
int rename_feature(char *project_path, char *old_name, char *new_name, char *error)
{
    // Make sure the old name is safe.
    int result = 0;
    if((result = check_valid_string(old_name, SAFE_CHARS_FILENAME)) != 0)
    {
        strncpy(error, "Invalid characters in old name of file.", MAX_ERROR);
        return result;
    }

    // Make sure the new name is safe.
    if((result = check_valid_string(new_name, SAFE_CHARS_FILENAME)) != 0)
    {
        strncpy(error, "Invalid characters in new name of file.", MAX_ERROR);
        return result;
    }

    // Get the full path for both new and old name
    char old_path[PATH_MAX+MAX_OBJECT_NAME+1] = "";
    char new_path[PATH_MAX+MAX_OBJECT_NAME+1] = "";
    snprintf(old_path, PATH_MAX+MAX_OBJECT_NAME, "%s/%s", project_path, old_name);
    snprintf(new_path, PATH_MAX+MAX_OBJECT_NAME, "%s/%s", project_path, new_name);

    // Try rename the feature
    if ((result = safe_rename(old_path, new_path)) != 0)
    {
        // Handle various errors
        if (result == ENOENT)
        {
            strncpy(error, "No such file or directory.", MAX_ERROR);
        }
        else
        {
            strncpy(error, "Unknown error occurred while trying to rename file.", MAX_ERROR);
        }
        return result;
    }
    return 0;
}


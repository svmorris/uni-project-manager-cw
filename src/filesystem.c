/*
 * This file deals with everything related to 
 * filesystem operations.
 *
 * Nothing outside this file should read
 * or write anything on the file-system,
 * thus limiting copies of potentially
 * vulnerable code.
 * This policy does make for some quite
 * redundant functions, but it makes more
 * sense for me to keep them in this file.
 *
 */

/*
 * NOTE: this code relies strongly on PATH_MAX
 * from `linux/limits.h`. At the time of writing
 * most of this I assumed that PATH_MAX was 
 * as described, the largest path you can have
 * in linux. However this is not true as this
 * rule is never enforced. It is a bit too late
 * to make it worth changing most of the code,
 * so try not to run it from an insanely long
 * path.
 * I don't think it could cause any security
 * issues, just some bugs under very extreme
 * circumstances.
 *
 * [Problems with PATH_MAX](https://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html)
 */




#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>

#include "util.h"
#include "filesystem.h"



// Default sub-directories of a project.
char DIRECTORIES[5][10] = {
    "src",
    "bin",
    "lib",
    "conf",
    "tests"
};


/*
 * Find the path to a file that has the name of `filename` and the contents of `contents`
 * 
 * Yes, this function is only useful for one thing, but its here because I want
 * to keep things that access the file-system in this one file.
 * 
 * Usage:
 * find_file_by_contents(
 *      base folder to search file in,
 *      filename to look for,
 *      contents of the file,
 * )
 */
bool find_file_by_contents(char * base_path, char * filename, char * contents, char *result_path)
{
    bool found = false;
    struct dirent *dir_entry;
    DIR *dir = opendir(base_path);

    // make sure basepath actually exists
    if (dir == NULL)
    {
        fprintf(stderr, "Could not open directory: %s ", base_path);
        perror(":");
        closedir(dir);
        return false;
    }

    FILE *fp;
    char path_found[PATH_MAX+1] = "";
    char full_filepath[PATH_MAX+1] = "";
    char tmp_file_content[MAX_OBJECT_NAME + 1] = "";
    // loop over each file or folder in the directory
    while ((dir_entry = readdir(dir)) != NULL)
    {
        // make sure its not . or ..
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
            continue;

        // get a path for the base_path + /current file
        snprintf(full_filepath, PATH_MAX, "%s/%s", base_path, dir_entry->d_name);

        // do different things on different file types
        switch (dir_entry->d_type)
        {
            // regular file
            case 8:
                if (strcmp(filename, dir_entry->d_name) != 0)
                    break;

                // try open file
                fp = fopen(full_filepath, "r");
                if (fp == NULL)
                {
                    fprintf(stderr, "Could not open file '%s' for reading", full_filepath);
                    break;
                }

                // Read 100 chars from the file to compare.
                // Yes, this only compares the first 100 chars,
                // but that is the maximum that can be written
                // into a tag anyway and I think that will be enough.
                fgets(tmp_file_content, MAX_OBJECT_NAME, fp);

                // This file is no longer needed
                fclose(fp);

                // If they are not equal then break the switch case
                // and go to next file.
                if (strncmp(tmp_file_content, contents, MAX_OBJECT_NAME) != 0)
                    break;

                // File found!
                // Copy the path to the found file into the
                // result path variable
                strncpy(result_path, full_filepath, PATH_MAX);

                // close everything
                closedir(dir);
                return true;


            // regular directory
            case 4:

                // In case of directories, check to see if the file is in any
                // of the sub-directories.
                found = find_file_by_contents(full_filepath, filename, contents, result_path);

                // The file was found in one of the sub-directories.
                if (found)
                {
                    closedir(dir);
                    return true;
                }

                // leave the switch case
                break;
        }

        // Null some buffers that are being reused
        // on each loop.
        memset(path_found, 0, PATH_MAX);
        memset(full_filepath, 0, PATH_MAX);
        memset(tmp_file_content, 0, MAX_OBJECT_NAME);
    }

    // I am pretty sure that some of these
    // closedir calls are redundant, but
    // I have no way of verifying that.
    closedir(dir);
    return false;
}


/*
 * system(ls -R);
 *
 * This function is a wrapper around the `ls` command
 * in bash. It just runs the command with a few arguments
 * and puts the output in the output_buffer.
 */
int system_list_dirs(char *base_path, int max_length, char *output_buffer)
{
    FILE *p;
    int ch;
    int i = 0;

    // Avoid any command injection
    if (check_valid_string(base_path, SAFE_CHARS_PATHNAME) != 0)
    {
        fprintf(stderr, "base_path includes invalid characters, can only include: '%s'", SAFE_CHARS_PATHNAME);
        return EINVAL;
    }

    // format ls command with `grep .` to remove unnecessary new lines and `grep -v '/$'` to
    // not show directories.
    char command[PATH_MAX+51] = "";
    // NOTE: hopefully check_valid_string has filtered enough characters to avoid command injection
    snprintf(command, PATH_MAX+50, "/bin/ls -FRa -I.git %s | grep . | grep -v '/$'", base_path);

    // Try run ls
    p = popen(command,"r");
    if( p == NULL)
    {
        // If something errors, its likely because
        // it could not find the ls command.
        perror("No such command: 'ls'");
        return ENOENT;
    }

    // get char-by-char
    while((ch = fgetc(p)) != EOF)
    {
        // make sure we don't read more
        // bytes than max_length
        if (i >= max_length)
            break;

        output_buffer[i] = ch;
        i ++;
    }
    pclose(p);
    return 0;
}


/*
 * Create default sub-directories of a project
 *
 * The created directories are denoted in
 * the `DIRECTORIES` variable.
 */
int create_default_sub_dirs(char *basepath)
{
    // NOTE: this is not a good implementation of this function
    // I am setting dirpath to PATH_MAX as it is the maximum path
    // allowed in linux and therefore can not overflow, however
    // this might be incredibly inefficient. (security > speed)
    char dirpath[PATH_MAX+11] = "";

    for(int i=0; i < 5; i++)
    {
        // format directory path as basepath/dir
        snprintf(dirpath, PATH_MAX+10, "%s/%s", basepath, DIRECTORIES[i]);
        /* printf("Sub: %s\n", dirpath); */

        // create directory and check error
        int ret = safe_mkdir(dirpath);
        if (ret != 0)
        {
            // If error occurred, print it and move on.
            // There is no need to crash as its just a
            // sub-directory.
            perror("Could not create subdirectory");
        }

        // reset the dirpath buffer to 0
        memset(dirpath, 0, PATH_MAX+10);
    }

    return 0;
}







/*
 * Function checks if a path already
 * exists.
 *
 * returns boolean.
 */
bool path_exists(char *path)
{
    struct stat s = {0};
    if (stat(path, &s) == 0)
    {
        return true;
    }
    return false;
}


/*
 * Function creates a new directory.
 *
 * Technically there is nothing "safe"
 * about this function, its mostly here
 * to comply with my philosophy of putting
 * all file-system related, potentially
 * unsafe code in one file.
 *
 */
int safe_mkdir(char * dir_path)
{
    if (path_exists(dir_path))
        return EEXIST;
    return mkdir(dir_path, NEW_FILE_PERMISSIONS);
}
/*
 * Function writes data to a file.
 *
 * Similarly to `safe_mkdir`, its not actually
 * any safer than doing it anywhere else,
 * just here to keep all file-system related
 * tasks in one file.
 */
int safe_write_file(char *file_path, char *file_content)
{
    if (path_exists(file_path))
        return EEXIST;


    FILE *fp;
    fp = fopen(file_path, "w");

    if (fp == NULL)
        // my best guess as to why it wouldn't work
        return EPERM;

    fprintf(fp, "%s", file_content);
    fclose(fp);

    return 0;
}



int safe_rename(char *old_path, char *new_path)
{
    if (!path_exists(old_path))
        return ENOENT;

    return rename(old_path, new_path);
}

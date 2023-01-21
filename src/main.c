#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>

#include "util.h"
#include "server.h"
#include "project.h"

int main(int argc, char **argv)
{
    // Other than create_project
    // all of the following need
    // a "base path" to operate
    // in.
    // This has to be explicitly
    // set because of the web-server
    // which will always run from
    // the same directory.
    char path_name[PATH_MAX];
    getcwd(path_name, PATH_MAX);

    // Buffer to hold errors created by the functions
    // in project.c. The reason this is not directly
    // put to stderr is that the web-server needs them
    // so it can return to the client.
    char error[MAX_ERROR+1] = "";
    int return_code = EINVAL;

    // Functions that take one argument
    if (argc == 3)
    {
        // Get the argument to the function
        // into its own variable. This allows
        // me to limit the size of the argument
        // once, instead of doing it in every
        // function.
        char object_name[MAX_OBJECT_NAME+1] = "";
        strncpy(object_name, argv[2], MAX_OBJECT_NAME);


        // pm create_project <project_name>
        if (strncmp(argv[1], "create_project", 14) == 0)
            return_code = create_project(object_name, error);


        // pm add_feature <feature_name>
        else if (strncmp(argv[1], "add_feature", 11) == 0)
            return_code = add_feature(object_name, path_name, error);


        // pm add_tag <tag_name>
        else if (strncmp(argv[1], "add_tag", 7) == 0)
            return_code = add_tag(object_name, path_name, error);


        // pm serve <port>
        else if (strncmp(argv[1], "serve", 5) == 0)
            return_code = serve(argv[0], object_name);


        // pm find_tag <tag_name>
        else if (strncmp(argv[1], "find_tag", 8) == 0)
        {
            // Because the web-server needs the output of
            // this function it has to have the response
            // put in a buffer instead of just printing to
            // stdout.
            char response[PATH_MAX+MAX_OBJECT_NAME+11] = "";

            return_code = find_tag(object_name, path_name, response);

            // Errors should be printed to stderr to better
            // follow the UNIX philosophy.
            if (return_code == 0)
                printf("%s\n", response);
            else
                fprintf(stderr, "%s\n", response);

            // Return here so help doesn't get printed
            // due to the lack of `error`.
            return return_code;

        }

    }
    // functions that need 2 arguments
    else if (argc == 4)
    {
        // Copy the arguments out to fix-size buffers
        // so we don't have to worry about argument sizes
        // inside project.c
        char argument1[MAX_OBJECT_NAME+1] = "";
        char argument2[MAX_OBJECT_NAME+1] = "";

        strncpy(argument1, argv[2], MAX_OBJECT_NAME);
        strncpy(argument2, argv[3], MAX_OBJECT_NAME);

        // pm rename_feature old_name new_name
        if (strncmp(argv[1], "rename_feature", 14) == 0)
            return_code = rename_feature(path_name, argument1, argument2, error);
    }

    // If there is a non 0 return code with no error
    // then the arguments were wrong and none of the
    // above functions ran
    if (return_code != 0 && strlen(error) == 0)
        crash_help(EINVAL);

    // Print any errors that the above functions
    // might have generated.
    if(return_code != 0)
        fprintf(stderr, "%s\n", error);

    // I try return useful error codes wherever possible
    return return_code;
}


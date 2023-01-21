#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


# define MAIN_HELPSTRING "Usage: pm [mode] <ARGUMENTS> PROJECT_NAME\n"\
                   "Create and manage projects.\n\n"\
                   "Available modes:\n"\
                   "\tcreate_project:\t\tCreate a new project.\n"\
                   "\tadd_feature:\t\tAdd feature to an existing project\n"\
                   "\tadd_tag:\t\tAdds a tag to a feature.\n"\
                   "\tfind_tag:\t\tLocate a tag within a project\n"\
                   "\nUse pm help [mode] for more information\n"


// Function prints the help message
// and exits.
void crash_help(int error_code)
{
    fprintf(stderr, MAIN_HELPSTRING);
    exit(error_code);
}


/*
 * Function makes sure that all characters in
 * in a char array are within the supplied
 * `allowed_charset`.
 */
int check_valid_string(char *path, char *allowed_charset)
{
    bool is_valid;

    // loop over the entire string
    for (int i = 0; path[i] != '\0'; i++)
    {
        is_valid = false;
        for (int j = 0; j < strlen(allowed_charset); j++)
        {
            if (allowed_charset[j] == path[i])
                is_valid = true;
        }

        if (!is_valid)
        {
            fprintf(stderr, "Invalid character '%c' used in pathname", path[i]);
            return EINVAL;
        }
    }
    return 0;
}

#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/limits.h>

#include "util.h"
#include "server.h"
#include "project.h"
#include "filesystem.h"


int init_server(char *runpath, int port)
{
    int opt = 1;
    int listen_fd, conn_fd, n;
    struct sockaddr_in server_address;
    char buffer[MAX_REQUEST_SIZE+1] = "";


    // Createa new internet socket
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Could not create socket");
        return EXIT_FAILURE;
    }

    // Zero out the address struct
    bzero(&server_address, sizeof(server_address));
    // set family
    server_address.sin_family      = AF_INET;
                                    // accept any requests
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port        = htons(port);

    // force server to reconnect to binded port
    // without this you have to change the port after server crashes
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    // bind address
    if ((bind(listen_fd, (SA *) &server_address, sizeof(server_address))) < 0)
    {
        perror("Could not bind listening address");
        return EXIT_FAILURE;
    }

    // start listening on the socket
    if ((listen(listen_fd, 10)) < 0)
    {
        perror("Error while listening for connections");
        return EXIT_FAILURE;
    }

    // Try make a large buffer for the response (html file)
    char *response_buffer = malloc(MAX_RESPONSE_SIZE+1);
    if (response_buffer == NULL)
    {
        fprintf(stderr, "Could not allocate heap memory");
        return ENOMEM;
    }

    printf("Listening on %d\n", port);
    // start listening in a loop for connections
    while (true)
    {
        // clear buffer
        fflush(stdout);

        // Accept connection
        conn_fd = accept(listen_fd, (SA *) NULL, NULL);

        // clear buffer for next read
        memset(buffer, 0, MAX_REQUEST_SIZE+1);

        // try read from socket
        // should put this on a new thread
        // I am not reding line-by-line as I dont see
        // the need for the extra complexity for this small
        // server.
        if ((n = read(conn_fd, buffer, MAX_REQUEST_SIZE)) < 0)
        {
            perror("Failed to read from socket");
            return EIO;
        }


        // Prepare response for sending back
        process_request(buffer, response_buffer);
        if (write(conn_fd, response_buffer, strlen(response_buffer)) != strlen(response_buffer))
        {
            perror("Failed to send response to client");
            memset(response_buffer, 0, MAX_REQUEST_SIZE);
            free(response_buffer);
            return EIO;
        }

        close(conn_fd);
        memset(response_buffer, 0, MAX_REQUEST_SIZE);

    }

    memset(response_buffer, 0, MAX_REQUEST_SIZE);
    free(response_buffer);
    return 0;
}

int process_request(char *buffer, char *response_buffer)
{
    // GET /path HTTP/1.1
    char method[11] = "";
    char path[101] = "";
    char schema[11] = "";

    // Try read the first 3 parts of the http request.
    // This first line should be enough to handle everything
    // this project manager needs.
    if(sscanf(buffer, "%10s %100s %10s", method, path, schema) != 3)
    {
        // failed to parse request
        fprintf(stderr, "[ INVALID REQUEST ] Ignoring.\n");
        strncpy(response_buffer, INVALID_REQUEST, 25);
        return EINVAL;
    }
    else // show a little log of the request
        printf("[ %s :: %s ]  %s\n",schema, method, path);


    // If index or index with a query parameter
    if (strcmp(path, "/") == 0 || strncmp(path, "/?", 2) == 0)
        web_index(response_buffer);

    else if (strncmp(path, "/create_project", 15) == 0)
        web_create_project(path, response_buffer);

    else if (strncmp(path, "/add_feature", 12) == 0)
        web_add_feature(path, response_buffer);

    else if (strncmp(path, "/rename_feature", 15) == 0)
        web_rename_feature(path, response_buffer);

    else if (strncmp(path, "/add_tag", 8) == 0)
        web_add_tag(path, response_buffer);

    else if (strncmp(path, "/find_tag", 9) == 0)
        web_find_tag(path, response_buffer);

    else
        strncpy(response_buffer, INVALID_REQUEST, 25);

    return 0;
}

int serve(char *argv0, char *port)
{
    int port_int = 0;
    // make sure argv0 is maximum PATH_MAX size
    char runpath[PATH_MAX+1] = "";
    strncpy(runpath, argv0, PATH_MAX);

    // try convert port to int
    port_int = atoi(port);

    // make sure range is correct
    if (1 > port_int || port_int > 65535)
    {
        fprintf(stderr, "Invalid port number, port can only be between 1 and 65536 (2^16)");
        return EINVAL;
    }

    // start server
    return init_server(runpath, port_int);
}


/*
 * Function parses out the URL parameter denoted
 * in `target` into the `value_buffer`.
 */
void get_url_parameter(char *url, char *target, char *value_buffer, int value_buffer_length)
{
    int url_length = strlen(url);

    // loop over all chars from the URL one-by-one
    for (int i = 0; i < url_length; i++)
    {
        // stop when it finds a URL parameter
        if (url[i] == '?' || url[i] == '&')
        {
            // check if the parameter found is the one that we are looking for
            if(strncmp(&url[i+1], target, strlen(target)) == 0 && url[i+strlen(target)+1] == '=')
            {
                // Set i to the start of the value
                // to make working with it easier.
                // As there is only one value we want to
                // get it doesn't really matter that this
                // ruins the rest of the loop.
                i = i+strlen(target)+2;

                // Using j to make sure we don't overflow
                // the value_buffer.
                int j = 0;
                while (j < value_buffer_length)
                {
                    // Only read till the next parameter, the end of
                    // the string or a colon representing the end of
                    // a directory name.
                    if (url[i] == '?' || url[i] == '&' || url[i] == ':' || url[i] == '\0')
                        break;

                    value_buffer[j] = url[i];

                    i++;
                    j++;
                }
            }
        }
    }
}


/*
 * Index page
 *
 * Function gets files, formats html
 * and puts the html for the webpage and
 * the http headers into the `response_buffer`
 * variable.
 */
void web_index(char *response_buffer)
{
    // Get tree output for the second box
    // on the page.
    int files_buffer_length = MAX_RESPONSE_SIZE - strlen(RESPONSE_HTML) - strlen(RESPONSE_HEADERS);
    char *files_buffer = malloc(files_buffer_length+1);
    memset(files_buffer, 0, files_buffer_length);

    // IMPORTANT: Because of the way this got implemented, the path here always
    // has to start with "./"
    system_list_dirs("./", files_buffer_length, files_buffer);


    // This is where the html formatted files buffer will be;
    int formatted_files_buffer_length = MAX_RESPONSE_SIZE-strlen(RESPONSE_HTML)-strlen(RESPONSE_HEADERS);
    char *formatted_files_buffer = malloc(formatted_files_buffer_length+1);
    memset(formatted_files_buffer, 0, formatted_files_buffer_length);


    // Write ./ for the top of the files list in the index window
    // TODO: add buttons to working directory.
    sprintf(formatted_files_buffer, "%s\r\n", GLOBAL_BUTTONS);
    // variable to count how many chars have been written to
    // formatted_files_buffer as I am concatenating strings to
    // it manually, which could cause buffer overflows.
    int chars_written = 5;

    /*
     * This following bit iterates over the string
     * line-by-line.
     *
     * The part where you can actually use the
     * data starts at `START:`
     */
    // Loop over each char to process it.
    int i = 0;
    int j = 0;
    int k = 0;
    char path_buffer[101] = "";
    bool first_char_in_line = false;
    char formatted_path_buffer[400+400+1] = "";
    while (files_buffer[i] != '\0')
    {
        // save i in j so we can iterate over j without
        // changing the value of i;
        j = i;
        k = 0;
        // if this is the first char in the line
        if (first_char_in_line)
        {
            // find end of line while adding each char to path_buffer
            while(files_buffer[j] != '\n')
            {
                path_buffer[k] = files_buffer[j];
                // add one to both of these
                j ++;
                k ++;
            }

            // START:
            // This part should be called at the beginning of each line

            /*
             * format directories to have buttons next to them and stuff like that
             */

            // Crude way to check if its directory, check if
            // the line starts with `./`
            if (strncmp(path_buffer, "./", 2) == 0)
            {
                // make html for button tag                            /// warning: ISO C does not support %n$ operand number formats [-Wformat=]
                snprintf(formatted_path_buffer, 400+400, HTML_DIR_BUTTONS, path_buffer, path_buffer, path_buffer, path_buffer);
            }
            else if (strcmp(path_buffer, "\n") == 0 || strlen(path_buffer) == 0)
            {
                strcpy(formatted_path_buffer, "");
            }
            else
            {
                // if it is not a directory, then no html is needed and it
                // can be displayed as just text
                snprintf(formatted_path_buffer, 400, "❯ %s\r\n", path_buffer);
            }


            // because of our manual implementation of this, we need to manually check if
            // the buffer is not being overrun by this strcat
            // make sure this would not overflow the allocated space for formatted_files_buffer
            if (formatted_files_buffer_length > chars_written+strlen(formatted_path_buffer))
                // concatenate part data to the formatted buffer
                strcat(formatted_files_buffer, formatted_path_buffer);
            else
            {
                // ran out of buffer space
                fprintf(stderr, "Could not display entire file tree");
                break;
            }

            // count all the new chars that have been written
            chars_written = chars_written + strlen(formatted_path_buffer);


            // delete the path_buffer for next turn.
            memset(path_buffer, 0, 100);
            memset(formatted_path_buffer, 0, 400);
        }



        // keeping track if its the first character
        // in a line to so we can check if its directory
        // or not.
        if (files_buffer[i] == '\n')
            first_char_in_line = true;
        else
            first_char_in_line = false;
        i ++;
    }

    // format the web  page
    char *tmp = malloc(MAX_RESPONSE_SIZE+1);
    snprintf(tmp, MAX_RESPONSE_SIZE, RESPONSE_HTML, formatted_files_buffer);

    // add headers to request
    snprintf(response_buffer, MAX_RESPONSE_SIZE, "%s\r\n\r\n%s", RESPONSE_HEADERS, tmp);

    // clear and free heap memory
    memset(formatted_files_buffer, 0, formatted_files_buffer_length);
    memset(files_buffer, 0, files_buffer_length);
    memset(tmp, 0, MAX_REQUEST_SIZE);
    free(formatted_files_buffer);
    free(files_buffer);
    free(tmp);
}

void web_create_project(char *request_path_and_arguments, char *response_buffer)
{
    // Parse project_name from URL parameter.
    char project_name[MAX_OBJECT_NAME+1] = "";
    get_url_parameter(request_path_and_arguments, "name", project_name, MAX_OBJECT_NAME);

    // create project, capturing return code in order to be able to show errors.
    char error[MAX_ERROR] = "";
    int return_code = create_project(project_name, error);

    // Redirect back to index, if there was an error than add the ?error parameter.
    if (return_code == 0)
        snprintf(response_buffer, MAX_RESPONSE_SIZE, HTTP_REDIRECT, "/");
    else
    {
        char error_param[MAX_ERROR+11] = "";
        snprintf(error_param, MAX_ERROR+10, "/?error=%s", error);
        snprintf(response_buffer, MAX_RESPONSE_SIZE, HTTP_REDIRECT, error_param);
    }
}

void web_add_feature(char *request_path_and_arguments, char *response_buffer)
{
    // Parse path from URL parameter.
    char path[101] = "";
    get_url_parameter(request_path_and_arguments, "path", path, 100);

    // Parse feature_name from URL parameter.
    char feature_name[MAX_OBJECT_NAME+1] = "";
    get_url_parameter(request_path_and_arguments, "name", feature_name, MAX_OBJECT_NAME);

    // create project, capturing return code in order to be able to show errors.
    char error[MAX_ERROR] = "";
    int return_code = add_feature(feature_name, path, error);

    // Redirect back to index, if there was an error than add the ?error parameter.
    if (return_code == 0)
        snprintf(response_buffer, MAX_RESPONSE_SIZE, HTTP_REDIRECT, "/");
    else
    {
        char error_param[MAX_ERROR+11] = "";
        snprintf(error_param, MAX_ERROR+10, "/?error=%s", error);
        snprintf(response_buffer, MAX_RESPONSE_SIZE, HTTP_REDIRECT, error_param);
    }
}

void web_add_tag(char *request_path_and_arguments, char *response_buffer)
{
    // Parse path from URL parameter.
    char path[101] = "";
    get_url_parameter(request_path_and_arguments, "path", path, 100);

    // Parse tag_name from URL parameter.
    char tag_name[MAX_OBJECT_NAME+1] = "";
    get_url_parameter(request_path_and_arguments, "name", tag_name, MAX_OBJECT_NAME);

    // add tag, capturing return code in order to be able to show errors.
    char error[MAX_ERROR] = "";
    int return_code = add_tag(tag_name, path, error);

    // Redirect back to index, if there was an error than add the ?error parameter.
    if (return_code == 0)
        snprintf(response_buffer, MAX_RESPONSE_SIZE, HTTP_REDIRECT, "/");
    else
    {
        char error_param[MAX_ERROR+11] = "";
        snprintf(error_param, MAX_ERROR+11, "/?error=%s", error);
        snprintf(response_buffer, MAX_RESPONSE_SIZE, HTTP_REDIRECT, error_param);
    }
}

void web_rename_feature(char *request_path_and_arguments, char *response_buffer)
{
    // parse path from URL parameter
    char path[101] = "";
    get_url_parameter(request_path_and_arguments, "path", path, 100);

    // Parse the new feature name from the URL parameters.
    char new_name[MAX_OBJECT_NAME+1] = "";
    get_url_parameter(request_path_and_arguments, "name", new_name, 100);

    // error buffer for holding errors
    char error[MAX_ERROR+1] = "";

    // Separate relative path from old feature name as
    // rename_feature needs it as separate arguments
    char old_name[MAX_OBJECT_NAME+1] = "";
    for (int i = strlen(path)-1; i >= 0; i--)
    {
        // Loop over path backwards, stop at first /
        // or at the end of the string
        if (path[i] == '/' || path[i] == '\0')
            break;

        // Copy the name into the old_name buffer starting from
        // the last /.
        // TODO: this could be neater
        strncpy(old_name, &path[i], MAX_OBJECT_NAME);
    }

    // get base path
    char base_path[PATH_MAX+1] = "";
    strncpy(base_path, path, strlen(path)-strlen(old_name));


    // If there have been no errors so far
    if (strlen(error) <= 0)
    {
        // Rename Feature
        int return_code = rename_feature(base_path, old_name, new_name, error);

        // If success: redirect back to index
        if (return_code == 0)
        {
            snprintf(response_buffer, MAX_RESPONSE_SIZE, HTTP_REDIRECT, "/");
            return;
        }
    }
    // If there was an error: redirect back with the error parameter.
    char error_param[MAX_ERROR+11] = "";
    snprintf(error_param, MAX_ERROR+11, "/?error=%s", error);
    snprintf(response_buffer, MAX_RESPONSE_SIZE, HTTP_REDIRECT, error_param);
}

void web_find_tag(char *request_path_and_arguments, char *response_buffer)
{
    char tag_name[MAX_OBJECT_NAME+1] = "";
    get_url_parameter(request_path_and_arguments, "name", tag_name, MAX_OBJECT_NAME);


    char response[PATH_MAX+MAX_OBJECT_NAME+11] = "";
    find_tag(tag_name, ".", response);

    // The response from find_tag should be displayed the same way
    // normal errors would, so I think it makes sense to just re-use
    // the ?error key word. I know this is a bit messy, but its only
    // in this one case.
    char error_param[PATH_MAX+MAX_OBJECT_NAME+22] = "";
    snprintf(error_param, PATH_MAX+MAX_OBJECT_NAME+20, "/?error=%s", response);
    snprintf(response_buffer, MAX_RESPONSE_SIZE, HTTP_REDIRECT, error_param);
}

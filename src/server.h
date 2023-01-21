# pragma once

# define SA struct sockaddr

# define MAX_REQUEST_SIZE 1024
# define MAX_RESPONSE_SIZE 1000001



# define INVALID_REQUEST "HTTP/2 400 Bad Request\r\n"
# define RESPONSE_HEADERS   "HTTP/2 200 OK\r\n"\
                            "Content-Type: text/html; charset=UTF-8\r\n"\

# define RESPONSE_HTML "<!DOCTYPE html>\r\n"\
                       "<html>\r\n"\
                           "<head>\r\n"\
                               "<meta charset='utf-8'>\r\n"\
                               "<title> Project Manager </title>\r\n"\
                           "</head>\r\n"\
                           "<script>\r\n"\
                               "let url_str = window.location.href;\r\n"\
                               "let url = new URL(url_str);\r\n"\
                               "let search_params = url.searchParams; \r\n"\
                               "let error = search_params.get('error');\r\n"\
                               "if (error != null)\r\n"\
                                   "alert(error);\r\n"\
                               "const createProject = () =>\r\n"\
                               "{\r\n"\
                                   "let project_name = prompt(`project name:`);\r\n"\
                                   "window.location.assign(`/create_project/?name=${project_name}`)\r\n"\
                               "}\r\n"\
                               "const findTag = () =>\r\n"\
                               "{\r\n"\
                                   "let tag_name = prompt(`tag name:`);\r\n"\
                                   "window.location.assign(`/find_tag/?name=${tag_name}`)\r\n"\
                               "}\r\n"\
                               "const addFeature = (path) =>\r\n"\
                               "{\r\n"\
                                   "let feature_name = prompt(`feature name:`);\r\n"\
                                   "window.location.assign(`/add_feature/?path=${path}&name=${feature_name}`)\r\n"\
                               "}\r\n"\
                               "const addTag = (path) =>\r\n"\
                               "{\r\n"\
                                   "let tag_name = prompt(`tag name:`);\r\n"\
                                   "window.location.assign(`/add_tag/?path=${path}&name=${tag_name}`)\r\n"\
                               "}\r\n"\
                               "const renameFeature = (path) =>\r\n"\
                               "{\r\n"\
                                   "let new_name = prompt(`new name:`);\r\n"\
                                   "window.location.assign(`/rename_feature/?path=${path}&name=${new_name}`)\r\n"\
                               "}\r\n"\
                           "</script>\r\n"\
                           "<style>\r\n"\
                               ":root { --light-accent: #737373; --accent: #000000; --text: #ffffef; --bg0: #ffffff; --bg1: #595959; font-family: 'Courier New'; font-size: 16px; color: var(--text); }\r\n"\
                               "html, body { margin: 0; padding: 0; height: 100%%; width: 100%%; background-color: var(--bg0); }\r\n"\
                               "header { font-size: 2em; padding: .2em; text-align: center; background-color: var(--bg1); }\r\n"\
                               "button { margin-left: 1em; margin-top: .3em; margin-bottom: .3em; font-size: 1em; padding-top: .2em; padding-bottom: .2em; padding-left: 1em; padding-right: 1em; border-radius: .3em;  border: solid .1em var(--accent);color: var(--text); background-color: var(--light-accent); }\r\n"\
                               "button:hover { color: var(--text); background-color: var(--bg1); }\r\n"\
                               ".box { margin: auto; margin-top: .5em; display: block; width: 95%%; min-height: 1.5em; max-height: 90vh; border-radius: .3em; border: solid .1em var(--accent); background-color: var(--bg1); overflow: scroll; position: absolute; top: 4em; bottom: 1em; left: 1em; right: 1em;}\r\n"\
                               ".box-inner { padding-top: .5em; padding-bottom: .5em; padding-left: 2em; padding-right: 2em; }\r\n"\
                               "a { text-decoration: none; color: inherit}\r\n"\
                               ".dir {display: flex;}\r\n"\
                           "</style>\r\n"\
                           "<body>\r\n"\
                               "<header>\r\n"\
                                   "Project Manager\r\n"\
                               "</header>\r\n"\
                               "<main>\r\n"\
                                   "<div class='box' id='command_output'>\r\n"\
                                       "<pre class='box-inner'>\r\n"\
                                           "%s\r\n"\
                                       "</pre>\r\n"\
                                   "</div>\r\n"\
                               "</main>\r\n"\
                           "</body>\r\n"\
                       "</html>\r\n"


# define GLOBAL_BUTTONS "<button onclick=createProject()> Create Project</button>"\
                        "<button onclick=findTag()> Find Tag</button>"\

# define HTML_DIR_BUTTONS "%s <div class='dir'>"\
                              "<button onclick=addFeature('%s')    > Add Feature </button>"\
                              "<button onclick=addTag('%s')        > Add Tag </button>"\
                              "<button onclick=renameFeature('%s') > Rename </button>"\
                          "</div>"

# define HTTP_REDIRECT "HTTP/2 302 Found\r\n"\
                             "Location: %s"

// general functions
int serve(char *argv0, char *port);
int init_server(char *runpath, int port);
int process_request(char *buffer, char *response_buffer);
void get_url_parameter(char *url, char *target, char *value_buffer, int value_buffer_length);

// pages
void web_index(char *response_buffer);
void web_add_tag(char *request_path_and_arguments, char *response_buffer);
void web_find_tag(char *request_path_and_arguments, char *response_buffer);
void web_add_feature(char *request_path_and_arguments, char *response_buffer);
void web_create_project(char *request_path_and_arguments, char *response_buffer);
void web_rename_feature(char *request_path_and_arguments, char *response_buffer);

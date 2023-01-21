# pragma once

#include <stdbool.h>


#define SAFE_CHARS_LEN 66

#define NEW_FILE_PERMISSIONS 0776




bool path_exists(char *path);
int safe_mkdir(char *dir_path);
int create_default_sub_dirs(char *basepath);
int safe_rename(char *old_path, char *new_path);
int safe_write_file(char *file_path, char *file_content);
int system_list_dirs(char *base_path, int max_length, char *output_buffer);
bool find_file_by_contents(char *base_path, char *filename, char * contents, char *result_path);


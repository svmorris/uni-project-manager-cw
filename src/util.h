# pragma once

#define MAX_ERROR 100
#define MAX_OBJECT_NAME 100

// These characters should be safe to use for filenames.
#define SAFE_CHARS_FILENAME "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_@."
#define SAFE_CHARS_PATHNAME "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_@./"

void crash_help(int error_code);
int check_valid_string(char *path, char *allowed_charset);

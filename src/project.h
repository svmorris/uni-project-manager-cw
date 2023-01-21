# pragma once

#define MAX_OBJECT_NAME 100

int create_project(char *project_name, char *error);

int add_feature(char *feature_name, char *project_path, char *error);
int rename_feature(char *project_path, char *old_name, char *new_name, char *error);

int add_tag(char *tag_name, char *project_path, char *error);
int find_tag(char *tag_name, char *prject_path, char *response);

#include "list_file.h"
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#define DEFAULT_BUFFER_SIZE 128

struct FileNames list_file(const char *directory_path) {
  struct FileNames file_names;

  DIR *dir = opendir(directory_path);
  if (dir == NULL) {
    file_names.length = -1;
    return file_names;
  }
  
  file_names.length = 0;
  int real_length = DEFAULT_BUFFER_SIZE;
  file_names.names = (char **)malloc(sizeof(char *) * real_length);

  struct dirent *dp;
  while ((dp = readdir(dir)) != NULL) {
    file_names.names[file_names.length] = (char *)malloc(sizeof(char) * strlen(dp->d_name));;
    strcpy(file_names.names[file_names.length], dp->d_name);

    file_names.length++;
    if (file_names.length == real_length) {
      real_length *= 2;
      file_names.names = (char **)realloc(file_names.names, sizeof(char *) * real_length);
    }
  }
  (void)closedir(dir);
  return file_names;
}

void free_file_names(struct FileNames file_names) {
  for (int i = 0; i < file_names.length; i++) {
    free(file_names.names[i]);
  }
  free(file_names.names);
}

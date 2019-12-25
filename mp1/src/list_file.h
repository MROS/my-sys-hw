#ifndef LIST_FILE_H
#define LIST_FILE_H

struct FileNames {
  int length;      // 代表擁多少個檔案， -1 表示開啓目錄失敗
  char **names;    // 以一個雙重指標表示 length 個檔名，每個檔名皆以 '\0' 爲結尾
};

// 若失敗則回傳值的 length 屬性爲 -1
// 成功則回傳 directory_path 代表目錄下所有檔案的 FileNames 結構
// 注意，會包含當前目錄"."以及其父目錄".."
struct FileNames list_file(const char *directory_path);

// 釋放一個 struct FileNames 所佔用的記憶體
void free_file_names(struct FileNames file_names);

#endif

/*
以下是一個使用該函式印出當前目錄下所有檔案檔名的範例程式

#include "list_file.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  struct FileNames file_names = list_file(".");
  for (size_t i = 0; i < file_names.length; i++) {
    printf("%s\n", file_names.names[i]);
  }
  free_file_names(file_names);
  return 0;
}
*/
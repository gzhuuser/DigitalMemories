#ifndef __READ_FOLD_H
#define __READ_FOLD_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>

// 函数声明
DIR *open_dir(const char *path);
bool isBMP(const char *filename);
bool isJPG(const char *filename);
struct dirent **filter_file(const char *path);

#endif // __READ_FOLD_H

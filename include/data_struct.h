#ifndef __DATA_STRUCT_H
#define __DATA_STRUCT_H

#include "kernel_list.h"
#include "show.h"
#include <stdbool.h>
#include <stdlib.h>

// 定义imgNode结构体
typedef struct imgNode
{
    int id;
    char *path;
    char *rgbData;
    char *type;
    bool isOpen;
    struct img_info info;
    struct list_head list;
} imgNode;

// 定义List结构体
typedef struct List
{
    struct list_head head;
    int size;
} List;

// 函数声明
List *initImgList();
imgNode *newNode(char *path, int id);
bool isEmpty(List *q);
bool List_add_tail(List *q, imgNode *node);
bool list_del_node(List *q, imgNode *node);
imgNode *findById(List *q, int id);
void printNode(const imgNode *node);
void printList(const List *q);

#endif // __DATA_STRUCT_H

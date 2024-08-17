#include "data_struct.h"

// 初始化图片列表
List *initImgList()
{
    List *q = (List *)malloc(sizeof(List));
    if (q != NULL)
    {
        INIT_LIST_HEAD(&q->head);
        q->size = 0;
    }
    return q;
}

// 创建新节点
imgNode *newNode(char *path, int id)
{
    imgNode *new = (imgNode *)calloc(1, sizeof(imgNode));
    if (new != NULL)
    {
        new->id = id;
        new->path = strdup(path);
        new->rgbData = NULL;
        new->isOpen = false;
        new->type = NULL;
        INIT_LIST_HEAD(&new->list);
    }
    return new;
}

// 判断列表是否为空
bool isEmpty(List *q)
{
    return q->size == 0;
}

// 将节点添加到列表尾部
bool List_add_tail(List *q, imgNode *node)
{
    list_add_tail(&node->list, &q->head);
    q->size++;
    return true;
}

// 从列表中删除节点
bool list_del_node(List *q, imgNode *node)
{
    if (isEmpty(q))
    {
        return false;
    }
    list_del_init(&node->list);
    q->size--;
    return true;
}

// 根据ID查找节点
imgNode *findById(List *q, int id)
{
    if (isEmpty(q))
    {
        return NULL;
    }
    struct list_head *pos;
    imgNode *entry;
    list_for_each(pos, &q->head)
    {
        entry = list_entry(pos, imgNode, list);
        if (entry->id == id)
        {
            return entry;
        }
    }
    return NULL;
}

// 打印节点信息
void printNode(const imgNode *node)
{
    if (node != NULL)
    {
        printf("ID: %d, Path: %s, Width: %d, Height: %d, BPP: %d, Is Open: %s, Type: %s\n",
               node->id, node->path, node->info.width, node->info.height, node->info.bpp,
               node->isOpen ? "true" : "false", node->type);
    }
}

// 打印列表中所有节点的信息
void printList(const List *q)
{
    struct list_head *pos;
    imgNode *entry;
    list_for_each(pos, &q->head)
    {
        entry = list_entry(pos, imgNode, list);
        printNode(entry);
    }
}

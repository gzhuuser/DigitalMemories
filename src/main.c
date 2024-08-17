#include "read_fold.h"
#include "data_struct.h"
#include "show.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "touch.h"

#include "client.h"

#define MAX 3

// 封装成函数
List *initFileList(const char *path)
{
    List *q = initImgList();
    struct dirent **filtered_files = filter_file(path);
    int count = 0;
    if (filtered_files != NULL)
    {
        for (int i = 0; filtered_files[i] != NULL; i++)
        {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, filtered_files[i]->d_name);

            printf("尝试打开文件: %s\n", full_path); // 打印完整路径进行调试

            imgNode *node = newNode(filtered_files[i]->d_name, i + 1);

            // 将jpg解码为RGB
            node->isOpen = false;
            if (isBMP(filtered_files[i]->d_name))
            {
                node->type = "BMP";
                printf("%s 是一个图片文件\n", filtered_files[i]->d_name);
                List_add_tail(q, node); // 确保节点成功加载后添加到链表
            }
            else if (isJPG(filtered_files[i]->d_name))
            {
                node->type = "JPG";
                printf("%s 是一个图片文件\n", filtered_files[i]->d_name);
                List_add_tail(q, node); // 确保节点成功加载后添加到链表
            }

            else
                printf("%s 不是一个图片文件\n", filtered_files[i]->d_name);
        }
    }
    else
    {
        printf("未找到任何文件\n");
    }

    return q;
}

int main(int argc, char **argv)
{
    const char *path = "./img"; // 目录路径
    char *background = "./phone1.jpg";

    // 打开触摸屏读取文件
    int tp = open("/dev/input/event0", O_RDWR);
    List *fileList = initFileList(path);
    // 只维持MAX个;
    int isOpened[MAX];

    // 初始化队列指针
    int front = 0, rear = 0;
    int count = 0; // 用来跟踪当前队列中元素的数量

    struct list_head *pos;
    imgNode *entry;

    printList(fileList);

    // 屏幕信息
    struct fb_var_screeninfo finfo;
    // 1. 准备好LCD设备 显存入口、分辨率……
    struct lcd_info linfo;
    init_lcd(&linfo, &finfo);
    // 初始化屏幕
    int n = 0;
    // 初始化背景
    init_background(background, &linfo, &finfo, 0);
    init_background(background, &linfo, &finfo, 1);
    init_background(background, &linfo, &finfo, 2);
    // 2. 显示指定JPEG图片到位置（0,0）
    init_lcd(&linfo, &finfo);

    chdir(path);

    int EVENT_TOUCH = 0;
    // 进入循环事件
    while (1)
    {
        printf("Initial event: %d\n", EVENT_TOUCH);

        // 确保初始位置指向第一个有效节点
        if (fileList->head.next != &fileList->head)
        {
            pos = fileList->head.next;
        }
        while (1)
        {
            printf("Waiting for event...\n");
            EVENT_TOUCH = click(tp);
            printf("Event: %d\n", EVENT_TOUCH);

            switch (EVENT_TOUCH)
            {
            case 1: // 打开下一张图片
            {
                do
                {
                    if (pos->next != &fileList->head)
                    {
                        pos = pos->next;
                    }
                    else
                    {
                        pos = fileList->head.next; // 循环到第一个节点
                    }
                    entry = list_entry(pos, imgNode, list);
                } while (entry->path == NULL);

                // 检查当前图片是否已经在isOpened数组中
                bool isAlreadyOpened = false;
                for (int i = 0; i < count; i++)
                {
                    if (isOpened[(front + i) % MAX] == entry->id)
                    {
                        isAlreadyOpened = true;
                        break;
                    }
                }

                if (!isAlreadyOpened)
                {
                    // 如果isOpened数组已满且当前图片不在isOpened数组中，释放最早打开的图片资源
                    if (count == MAX)
                    {
                        imgNode *entry_temp = findById(fileList, isOpened[front]);
                        if (entry_temp && entry_temp->isOpen)
                        {
                            free(entry_temp->rgbData);
                            entry_temp->isOpen = false;
                        }
                        front = (front + 1) % MAX; // 更新front指针，指向下一个最早打开的图片
                        count--;                   // 队列中有效元素减少一个
                    }

                    // 打开新的图片
                    if (!entry->isOpen)
                    {
                        char *jpgdata = load_img(entry->path, &entry->info);
                        entry->isOpen = true;
                        // jpg2rgb(jpgdata, entry->info.img_size, &entry->info);
                    }

                    // 记录新的图片ID到isOpened数组，并更新rear指针
                    isOpened[rear] = entry->id;
                    rear = (rear + 1) % MAX;
                    count++; // 队列中有效元素增加一个
                }

                // 显示图片
                printf("entry->path: %s\n", entry->path);
                init_lcd(&linfo, &finfo);
                if (isJPG(entry->path))
                {
                    showImage(entry->path, 258, 120, &linfo, &finfo, n, false);
                }
                else if (isBMP(entry->path))
                {
                    showImageBMP(entry->path, 258, 120, &linfo, &finfo, n, false);
                }

                printList(fileList);
                n++;
                break;
            }
            case 2: // 打开上一张图片
            {
                do
                {
                    if (pos->prev != &fileList->head)
                    {
                        pos = pos->prev;
                    }
                    else
                    {
                        pos = fileList->head.prev; // 循环到最后一个节点
                    }
                    entry = list_entry(pos, imgNode, list);
                } while (entry->path == NULL);

                // 检查当前图片是否已经在isOpened数组中
                bool isAlreadyOpened = false;
                for (int i = 0; i < count; i++)
                {
                    if (isOpened[(front + i) % MAX] == entry->id)
                    {
                        isAlreadyOpened = true;
                        break;
                    }
                }

                if (!isAlreadyOpened)
                {
                    // 如果isOpened数组已满且当前图片不在isOpened数组中，释放最后打开的图片资源
                    if (count == MAX)
                    {
                        // rear 指针指向的应该是下一个插入的位置，所以最后的元素在 (rear - 1 + MAX) % MAX
                        int last = (rear - 1 + MAX) % MAX;
                        imgNode *entry_temp = findById(fileList, isOpened[last]);
                        if (entry_temp && entry_temp->isOpen)
                        {
                            free(entry_temp->rgbData);
                            entry_temp->isOpen = false;
                        }
                        rear = last; // 更新 rear 指针指向最后的有效元素
                        count--;     // 队列中有效元素减少一个
                    }

                    // 打开新的图片
                    if (!entry->isOpen)
                    {
                        char *jpgdata = load_img(entry->path, &entry->info);
                        entry->isOpen = true;
                        // jpg2rgb(jpgdata, entry->info.img_size, &entry->info);
                    }

                    // 记录新的图片ID到isOpened数组，并更新front指针
                    front = (front - 1 + MAX) % MAX;
                    isOpened[front] = entry->id;
                    count++; // 队列中有效元素增加一个
                }

                // 显示图片
                printf("entry->path: %s\n", entry->path);
                init_lcd(&linfo, &finfo);
                if (isJPG(entry->path))
                {
                    showImage(entry->path, 258, 120, &linfo, &finfo, n, false);
                }
                else if (isBMP(entry->path))
                {
                    showImageBMP(entry->path, 258, 120, &linfo, &finfo, n, false);
                }
                printList(fileList);
                n++;
                break;
            }

            case 3: // 根据服务器返回的图片序号显示图片
            {
                // 1. 开始录音
                system("arecord -d5 -c1 -r16000 -twav -fS16_LE 1.wav");

                // 2. 发送录音文件给服务器，并获取服务器返回的图片序号
                int index = send_wav(); // 假设 send_wav() 函数返回图片的 index
                // int index = 1;
                printf("index:%d\n", index);

                // 3. 查找与 index 对应的图片节点
                imgNode *entry_1 = NULL;
                struct list_head *start_pos = pos; // 保存开始查找的初始位置
                do
                {
                    entry_1 = list_entry(pos, imgNode, list);
                    printf("%d \n", entry_1->id);
                    if (entry_1->id == index)
                    {
                        // 找到匹配的图片
                        printf("找到匹配的图片了\n");
                        break;
                    }

                    pos = pos->next;            // 移动到下一个节点
                    if (pos == &fileList->head) // 如果到达末尾，循环回到链表头
                    {
                        pos = fileList->head.next;
                    }

                } while (pos != start_pos); // 当重新回到开始查找的地方时停止

                if (entry_1 && entry_1->id == index)
                {
                    printf("开始渲染\n");
                    // 检查当前图片是否已经在isOpened数组中
                    bool isAlreadyOpened = false;
                    for (int i = 0; i < count; i++)
                    {
                        if (isOpened[(front + i) % MAX] == entry_1->id)
                        {
                            isAlreadyOpened = true;
                            break;
                        }
                    }

                    if (!isAlreadyOpened)
                    {
                        // 如果isOpened数组已满且当前图片不在isOpened数组中，释放最早打开的图片资源
                        if (count == MAX)
                        {
                            imgNode *entry_temp = findById(fileList, isOpened[front]);
                            if (entry_temp && entry_temp->isOpen)
                            {
                                free(entry_temp->rgbData);
                                entry_temp->isOpen = false;
                            }
                            front = (front + 1) % MAX; // 更新front指针，指向下一个最早打开的图片
                            count--;                   // 队列中有效元素减少一个
                        }

                        // 打开新的图片
                        if (!entry_1->isOpen)
                        {
                            char *jpgdata = load_img(entry_1->path, &entry_1->info);
                            entry_1->isOpen = true;
                            jpg2rgb(jpgdata, entry_1->info.img_size, &entry_1->info);
                        }

                        // 记录新的图片ID到isOpened数组，并更新rear指针
                        isOpened[rear] = entry_1->id;
                        rear = (rear + 1) % MAX;
                        count++; // 队列中有效元素增加一个
                    }

                    // 显示找到的图片
                    printf("entry->path: %s\n", entry_1->path);
                    init_lcd(&linfo, &finfo);
                    if (isJPG(entry_1->path))
                    {
                        showImage(entry_1->path, 258, 120, &linfo, &finfo, n, false);
                    }
                    else if (isBMP(entry_1->path))
                    {
                        showImageBMP(entry_1->path, 258, 120, &linfo, &finfo, n, false);
                    }

                    printList(fileList);
                    n++;
                }
                else
                {
                    printf("未找到匹配的图片序号: %d\n", index);
                }
                break;
            }
            case 4: // 退出程序
            {
                chdir("../");
                init_background("./black.jpg", &linfo, &finfo, 2);
                return 0;
            }
            default:
                printf("Unhandled event: %d\n", EVENT_TOUCH); // 调试信息
                break;
            }
        }
    }

    return 0;
}
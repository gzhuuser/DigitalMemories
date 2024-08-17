#include "read_fold.h"

// 打开目录
DIR *open_dir(const char *path)
{
    DIR *dp = opendir(path);
    if (dp == NULL)
    {
        perror("打开文件夹失败");
        return NULL;
    }
    return dp;
}

// 判断文件是否为BMP格式
bool isBMP(const char *filename)
{
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
    {
        return false;
    }

    // 比较后缀
    return strcasecmp(dot + 1, "bmp") == 0;
}

// 判断文件是否为JPG或JPEG格式
bool isJPG(const char *filename)
{
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
    {
        return false;
    }

    // 比较后缀
    return strcasecmp(dot + 1, "jpg") == 0 || strcasecmp(dot + 1, "jpeg") == 0;
}

// 过滤文件夹中的图片文件
struct dirent **filter_file(const char *path)
{
    DIR *dir = open_dir(path);

    if (dir == NULL)
    {
        return NULL;
    }

    struct dirent **imgs = NULL;
    int count = 0;
    int capacity = 10;

    imgs = malloc(capacity * sizeof(struct dirent *));
    if (imgs == NULL)
    {
        perror("malloc");
        closedir(dir);
        return NULL;
    }

    for (;;)
    {
        struct dirent *ep = readdir(dir);
        if (ep == NULL)
        {
            break;
        }

        if (isBMP(ep->d_name) || isJPG(ep->d_name))
        {
            if (count >= capacity)
            {
                capacity *= 2;
                struct dirent **tmp = realloc(imgs, capacity * sizeof(struct dirent *));
                if (tmp == NULL)
                {
                    perror("realloc");
                    free(imgs);
                    closedir(dir);
                    return NULL;
                }
                imgs = tmp;
            }
            imgs[count++] = ep;
        }
    }

    if (count >= capacity)
    {
        struct dirent **tmp = realloc(imgs, (capacity + 1) * sizeof(struct dirent *));
        if (tmp == NULL)
        {
            perror("realloc");
            free(imgs);
            closedir(dir);
            return NULL;
        }
        imgs = tmp;
    }
    imgs[count] = NULL;

    closedir(dir);
    return imgs;
}

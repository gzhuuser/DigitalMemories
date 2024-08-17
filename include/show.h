#ifndef __SHOW_H
#define __SHOW_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/input.h>
#include "jpeglib.h"

// 定义结构体
struct bitmap_header
{
    int16_t type;
    int32_t size;
    int16_t reserved1;
    int16_t reserved2;
    int32_t offbits;
} __attribute__((packed));

struct bitmap_info
{
    int32_t size;
    int32_t width;
    int32_t height;
    int16_t planes;
    int16_t bit_count;
    int32_t compression;
    int32_t size_img;
    int32_t X_pel;
    int32_t Y_pel;
    int32_t clrused;
    int32_t clrImportant;
} __attribute__((packed));

// LCD关联结构体
struct lcd_info
{
    int lcd;
    int width;
    int height;
    int bpp;
    char *fbm;
};

// 图像相关结构体
struct img_info
{
    FILE *fp;
    int width;
    int height;
    int bpp;
    int img_size;
    char *RGB;
};

// 函数声明
void init_lcd(struct lcd_info *p, struct fb_var_screeninfo *finfo);
void release_lcd(struct lcd_info *p);
char *load_img(const char *jpgFile, struct img_info *p);
void jpg2rgb(const char *jpgdata, size_t jpgsize, struct img_info *p);
void average_pooling_resize(struct img_info *src, struct img_info *dst, int new_width, int new_height);
void getBmpInfo(int fd, struct bitmap_header *header, struct bitmap_info *info, char **rgb);
void average_pooling_resize_bmp(struct img_info *src, struct img_info *dst, int new_width, int new_height);
void showImageBMP(const char *bmpFile, int x, int y, struct lcd_info *p, struct fb_var_screeninfo *vinfo, int n, bool background);
void showImage(const char *jpgFile, int x, int y, struct lcd_info *p, struct fb_var_screeninfo *vinfo, int n, bool background);
void init_background(const char *path, struct lcd_info *linfo, struct fb_var_screeninfo *finfo, int n_screen);

#endif // __SHOW_H

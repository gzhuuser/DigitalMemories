#include "show.h"

void init_lcd(struct lcd_info *p, struct fb_var_screeninfo *finfo)
{
    if (p == NULL || finfo == NULL)
        return;

    static bool initialized = false;
    if (initialized)
        return;

    p->lcd = open("/dev/fb0", O_RDWR);
    if (p->lcd == -1)
    {
        perror("打开液晶屏失败");
        return;
    }

    if (ioctl(p->lcd, FBIOGET_VSCREENINFO, finfo) != 0)
    {
        perror("获取屏幕参数失败");
        return;
    }
    p->width = finfo->xres;
    p->height = finfo->yres;
    p->bpp = finfo->bits_per_pixel;

    int screen_size = 3 * p->width * p->height * p->bpp / 8;
    p->fbm = mmap(NULL, screen_size, PROT_WRITE, MAP_SHARED, p->lcd, 0);

    if (p->fbm == NULL)
    {
        perror("内存分配失败");
        return;
    }
    bzero(p->fbm, screen_size);
    initialized = true;
}

void release_lcd(struct lcd_info *p)
{
    if (p == NULL)
    {
        return;
    }

    munmap(p->fbm, p->width * p->height * p->bpp / 8);
    close(p->lcd);
}

char *load_img(const char *jpgFile, struct img_info *p)
{
    if (jpgFile == NULL)
        return NULL;

    p->fp = fopen(jpgFile, "rb");
    if (p->fp == NULL)
    {
        perror("打开jpg图片失败");
        return NULL;
    }

    fseek(p->fp, 0, SEEK_END);
    p->img_size = ftell(p->fp);
    fseek(p->fp, 0, SEEK_SET);

    char *jpgdata = calloc(1, p->img_size);

    while (1)
    {
        int n = fread(jpgdata, p->img_size, 1, p->fp);

        if (n == 1)
            continue;

        if (n < 1)
        {
            if (feof(p->fp))
                break;

            if (ferror(p->fp))
            {
                perror("加载图片失败");
                fclose(p->fp);
                free(jpgdata);
                return NULL;
            }
        }
    }

    return jpgdata;
}

void jpg2rgb(const char *jpgdata, size_t jpgsize, struct img_info *p)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, jpgdata, jpgsize);

    if (!jpeg_read_header(&cinfo, true))
    {
        fprintf(stderr, "jpeg_read_header failed: %s\n", strerror(errno));
        return;
    }

    jpeg_start_decompress(&cinfo);
    p->width = cinfo.output_width;
    p->height = cinfo.output_height;
    p->bpp = cinfo.output_components * 8;

    unsigned long linesize = cinfo.output_width * cinfo.output_components;
    unsigned long rgbsize = linesize * cinfo.output_height;
    p->RGB = calloc(1, rgbsize);

    while (cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *buffer_array[1];
        buffer_array[0] = p->RGB + cinfo.output_scanline * linesize;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}

void average_pooling_resize(struct img_info *src, struct img_info *dst, int new_width, int new_height)
{
    int x_ratio = (int)((src->width << 16) / new_width) + 1;
    int y_ratio = (int)((src->height << 16) / new_height) + 1;

    dst->width = new_width;
    dst->height = new_height;
    dst->bpp = src->bpp;
    dst->img_size = new_width * new_height * src->bpp / 8;
    dst->RGB = (char *)malloc(dst->img_size);
    printf("开始进行缩放\n");

    for (int i = 0; i < new_height; i++)
    {
        for (int j = 0; j < new_width; j++)
        {
            int x = (j * x_ratio) >> 16;
            int y = (i * y_ratio) >> 16;
            int pixel_index = (i * new_width + j) * 3;
            int src_index = (y * src->width + x) * 3;

            dst->RGB[pixel_index] = src->RGB[src_index];
            dst->RGB[pixel_index + 1] = src->RGB[src_index + 1];
            dst->RGB[pixel_index + 2] = src->RGB[src_index + 2];
        }
    }
}

void getBmpInfo(int fd, struct bitmap_header *header, struct bitmap_info *info, char **rgb)
{
    read(fd, header, sizeof(*header));
    read(fd, info, sizeof(*info));
    printf("宽度: %d, 高度: %d, 色深: %d\n", info->width, info->height, info->bit_count / 8);
    *rgb = (char *)malloc(info->width * info->height * info->bit_count / 8);
    if (*rgb == NULL)
    {
        perror("分配内存失败");
        exit(1);
    }
    read(fd, *rgb, info->width * info->height * info->bit_count / 8);
}

void average_pooling_resize_bmp(struct img_info *src, struct img_info *dst, int new_width, int new_height)
{
    int x_ratio = (int)((src->width << 16) / new_width) + 1;
    int y_ratio = (int)((src->height << 16) / new_height) + 1;

    dst->width = new_width;
    dst->height = new_height;
    dst->bpp = src->bpp;
    dst->img_size = new_width * new_height * dst->bpp;
    dst->RGB = (char *)malloc(dst->img_size);
    if (dst->RGB == NULL)
    {
        perror("分配内存失败");
        exit(1);
    }

    printf("开始进行缩放\n");
    for (int i = 0; i < new_height; i++)
    {
        for (int j = 0; j < new_width; j++)
        {
            int x = (j * x_ratio) >> 16;
            int y = (i * y_ratio) >> 16;
            int pixel_index = (i * new_width + j) * dst->bpp;
            int src_index = (y * src->width + x) * src->bpp;

            if (pixel_index >= dst->img_size || src_index >= src->img_size)
            {
                printf("索引超出范围: pixel_index = %d, src_index = %d\n", pixel_index, src_index);
                exit(1);
            }

            memcpy(&dst->RGB[pixel_index], &src->RGB[src_index], dst->bpp);
        }
    }
}

void showImageBMP(const char *bmpFile, int x, int y, struct lcd_info *p, struct fb_var_screeninfo *vinfo, int n, bool background)
{
    int fd = open(bmpFile, O_RDONLY);
    if (fd == -1)
    {
        perror("打开BMP文件失败");
        return;
    }

    struct bitmap_header header;
    struct bitmap_info info;
    char *rgb = NULL;
    getBmpInfo(fd, &header, &info, &rgb);
    printf("成功获取BMP图片\n");
    close(fd);

    struct img_info minfo;
    minfo.width = info.width;
    minfo.height = info.height;
    minfo.bpp = info.bit_count / 8;
    minfo.img_size = minfo.width * minfo.height * minfo.bpp;
    minfo.RGB = rgb;

    struct img_info resized_minfo;
    printf("进入缩放阶段\n");
    if (background)
    {
        resized_minfo = minfo;
    }
    else
    {
        average_pooling_resize_bmp(&minfo, &resized_minfo, 292, 299);
    }

    printf("成功缩放BMP图片\n");
    int screen_size = p->width * p->height * p->bpp / 8;
    char *tmp = p->fbm + screen_size * (n % 3) + p->width * p->bpp / 8 * y + x * p->bpp / 8;

    for (int j = 0; j < resized_minfo.height && j < p->height - y; j++)
    {
        int moffset = resized_minfo.width * resized_minfo.bpp * (resized_minfo.height - 1 - j);
        int loffset = p->width * p->bpp / 8 * j;

        for (int i = 0; i < resized_minfo.width && i < p->width - x; i++)
        {
            memcpy(tmp + 4 * i + loffset + 0, resized_minfo.RGB + 3 * i + moffset + 0, 1);
            memcpy(tmp + 4 * i + loffset + 1, resized_minfo.RGB + 3 * i + moffset + 1, 1);
            memcpy(tmp + 4 * i + loffset + 2, resized_minfo.RGB + 3 * i + moffset + 2, 1);
            tmp[4 * i + loffset + 3] = 0;
        }
    }

    vinfo->yoffset = p->height * (n % 3);
    if (ioctl(p->lcd, FBIOPAN_DISPLAY, vinfo) == -1)
    {
        perror("FBIOPAN_DISPLAY");
    }

    if (!background)
    {
        free(resized_minfo.RGB);
    }
    free(minfo.RGB);
}

void showImage(const char *jpgFile, int x, int y, struct lcd_info *p, struct fb_var_screeninfo *vinfo, int n, bool background)
{
    struct img_info minfo;
    char *jpgdata = load_img(jpgFile, &minfo);

    if (isJPG(jpgFile))
    {
        jpg2rgb(jpgdata, minfo.img_size, &minfo);
    }
    else
    {
        return;
    }

    printf("jgpsize: %d\n", minfo.img_size);
    printf("jpg 尺寸: %d×%d\n\n", minfo.width, minfo.height);
    printf("----------------------------------------------------------------------------\n");

    struct img_info resized_minfo;
    if (background)
    {
        resized_minfo = minfo;
    }
    else
    {
        average_pooling_resize(&minfo, &resized_minfo, 292, 299);
    }

    int screen_size = p->width * p->height * p->bpp / 8;
    char *tmp = p->fbm + screen_size * (n % 3) + p->width * p->bpp / 8 * y + x * p->bpp / 8;

    for (int j = 0; j < resized_minfo.height && j < p->height - y; j++)
    {
        int moffset = resized_minfo.width * resized_minfo.bpp / 8 * j;
        int loffset = p->width * p->bpp / 8 * j;
        for (int i = 0; i < resized_minfo.width && i < p->width - x; i++)
        {
            memcpy(tmp + 4 * i + loffset + 0, resized_minfo.RGB + 3 * i + moffset + 2, 1);
            memcpy(tmp + 4 * i + loffset + 1, resized_minfo.RGB + 3 * i + moffset + 1, 1);
            memcpy(tmp + 4 * i + loffset + 2, resized_minfo.RGB + 3 * i + moffset + 0, 1);
        }
    }

    vinfo->yoffset = 480 * (n % 3);
    if (ioctl(p->lcd, FBIOPAN_DISPLAY, vinfo) == -1)
    {
        perror("FBIOPAN_DISPLAY");
    }

    free(jpgdata);
    if (!background)
    {
        free(resized_minfo.RGB);
    }
    free(minfo.RGB);
}

void init_background(const char *path, struct lcd_info *linfo, struct fb_var_screeninfo *finfo, int n_screen)
{
    showImage(path, 0, 0, linfo, finfo, n_screen, true);
}

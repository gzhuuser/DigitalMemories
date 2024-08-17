#ifndef __TOUCH_H
#define __TOUCH_H

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/input.h>

// 函数声明
int click(int tp);

#endif // __TOUCH_H

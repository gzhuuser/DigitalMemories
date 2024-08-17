#include "touch.h"

// 实现点击事件处理
int click(int tp)
{
    int x1 = 0, x2 = 0;
    int y1 = 0, y2 = 0;

    bool x1done = false;
    bool y1done = false;

    struct input_event buf;
    while (1)
    {
        bzero(&buf, sizeof(buf));
        int bytesRead = read(tp, &buf, sizeof(buf));
        if (bytesRead < sizeof(buf))
        {
            perror("Failed to read input event");
            continue;
        }

        // 防止抖动
        if (buf.type == EV_ABS && buf.code == ABS_X && !x1done)
        {
            x2 = x1 = buf.value * 800 / 1024;
            x1done = true;
        }

        if (buf.type == EV_ABS && buf.code == ABS_Y && !y1done)
        {
            y2 = y1 = buf.value * 800 / 1024;
            y1done = true;
        }

        if (buf.type == EV_ABS && buf.code == ABS_X)
            x2 = buf.value * 800 / 1024;
        if (buf.type == EV_ABS && buf.code == ABS_Y)
            y2 = buf.value * 800 / 1024;

        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            // 在离开屏幕时打印调试信息
            printf("x1:%d x2:%d y1:%d y2:%d \n", x1, x2, y1, y2);

            // 离开屏幕后,压力值为0
            // 判断点击的按钮
            if (abs(x2 - x1) > 150 && abs(y2 - y1) < 100)
            {
                if (x2 < x1)
                {
                    // 左滑动
                    return 1;
                }
                else
                {
                    // 右滑动
                    return 2;
                }
            }
            else if (x2 > 591 && x2 < 733 && y2 > 90 && y2 < 150)
            {
                return 1;
            }
            else if (x2 > 590 && x2 < 733 && y2 > 175 && y2 < 215)
            {
                return 2;
            }
            else if (x2 > 590 && x2 < 733 && y2 > 240 && y2 < 280)
            {
                return 3;
            }
            else if (x2 > 590 && x2 < 733 && y2 > 311 && y2 < 351)
            {
                return 4;
            }
            else
            {
                x1done = false;
                y1done = false;
                continue;
            }
        }
    }
}

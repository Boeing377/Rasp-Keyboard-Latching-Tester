#include "kb_ev_det.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>

struct kb_ev_det_t
{
    int fd;
    int event_num;
    int det;
    int value;
};

int *waitForKeyboardEvent(struct kb_ev_det_t *arg)
{
    struct input_event ev;
    while (arg->det == 0)
    {
        read(arg->fd, &ev, sizeof(struct input_event));
        if (ev.type == 1)
        {
            arg->value = ev.code;
            arg->det = 1;
        }
    }
    return &arg->value;
}

int numOfEvents()
{
    // 获取/dev/input/目录下event*文件的数量
    // 通过ls /dev/input/ | grep event | wc -l命令获取

    FILE *fp;
    int count = 0;
    char path[1035];

    // Run the command to get the number of event* files
    fp = popen("ls /dev/input/ | grep event | wc -l", "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        return 0;
    }

    // Read the output a line at a time - output it.
    if (fgets(path, sizeof(path) - 1, fp) != NULL)
    {
        count = atoi(path);
    }

    // Close the file pointer
    pclose(fp);

    return count;
}

struct kb_ev_det_result_t kbEventDetect()
{
    int fd[MAX_EV_NUM][2];
    int i;
    // int ev_num = 0;
    // int ev_key = 0;
    struct kb_ev_det_result_t result;

    result.event_num = -1;
    result.value = 0;

    int count = numOfEvents();
    if (count == 0)
    {
        printf("没要找到事件位于/dev/input/\n");
        printf("请检查键盘是否接入\n");
        return result;
    }
    // 打印事件数量
    printf("共有事件数量: %d\t", count);

    for (i = 0; i < count; i++)
    {
        char path[20];
        sprintf(path, "/dev/input/event%d", i);
        fd[i][0] = open(path, O_RDONLY);
        if (fd[i] < 0)
        {
            printf("打开%s失败\n", path);
            fd[i][1] = 0;
        }
        else
        {
            fd[i][1] = 1;
        }
    }

    struct kb_ev_det_t arg[MAX_EV_NUM];
    pthread_t tid[MAX_EV_NUM];

    for (i = 0; i < count; i++)
    {
        if (fd[i][1] == 1)
        {
            arg[i].fd = fd[i][0];
            arg[i].event_num = i;
            arg[i].det = 0;
            arg[i].value = 0;
            pthread_create(&tid[i], NULL, (void *)waitForKeyboardEvent, &arg[i]);
        }
    }

    while (1)
    {
        for (i = 0; i < count; i++)
        {
            if (arg[i].det == 1)
            {
                // printf("事件%d触发\n", arg[i].event_num);
                result.event_num = arg[i].event_num;
                result.value = arg[i].value;
                break;
            }
        }
        if (result.event_num != -1)
        {
            for (i = 0; i < count; i++)
            {
                if (fd[i][1] == 1)
                {
                    pthread_cancel(tid[i]);
                    close(fd[i][0]);
                }
            }
            break;
        }
    }

    return result;
}

void kb_Event_Detect(int event_num)
{
    int fd;
    struct input_event ev;
    int res;
    char path[20];
    sprintf(path, "/dev/input/event%d", event_num);
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        printf("打开%s失败\n", path);
        return;
    }

    while (1)
    {
        res = read(fd, &ev, sizeof(struct input_event));
        if (res == -1)
        {
            close(fd);
            fd = open(path, O_RDONLY);
            if (fd < 0)
            {
                printf("打开%s失败\n", path);
                return;
            }
        }
        if (ev.type == 1)
        {
            if (ev.value == 1)
            {
                printf("按下键盘按键%d\n", ev.code);
            }
            else if (ev.value == 0)
            {
                printf("松开键盘按键%d\n", ev.code);
            }
        }
    }
}
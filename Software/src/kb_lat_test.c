#include "kb_lat_test.h"
#include "result_data_processing.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>

sem_t event_read_rdy_sem;
sem_t key_singal_sem;
sem_t key_detect_sem;

unsigned long timestamp_key_trigger_signal;
unsigned long timestamp_key_release_signal;
unsigned long timestamp_key_trigger_detect;
unsigned long timestamp_key_release_detect;
unsigned long timestamp_overtime_detect;

unsigned long key_trigger_latency;
unsigned long key_release_latency;

int key_trigger_detect_flag = 0;
int key_release_detect_flag = 0;
int timeout_times = 0;
int timeout_flag = 0;
int trigger_timeout_flag = 0;
int release_timeout_flag = 0;

int trigger_test_flag = 0;
int release_test_flag = 0;

int error_flag = 0;

unsigned long get_timestamp_us()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

struct event_arg
{
    int event_num;
    int event_value;
};

int kbEventReadThread(void *arg)
{
    struct event_arg *ev_arg = (struct event_arg *)arg;
    char KB_EVENT[40];
    sprintf(KB_EVENT, "/dev/input/event%d", ev_arg->event_num);
    int fd = open(KB_EVENT, O_RDONLY);
    struct input_event ev;
    if (fd < 0)
    {
        printf("打开%s失败\n", KB_EVENT);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    fd_set set;
    struct timeval timeout;
    // struct timespec sem_timeout;

    int key_event_detect_flag = 0;
    int ret;

    timeout.tv_sec = MAX_ALLOW_TIMEOUT_MS / 1000;
    timeout.tv_usec = MAX_ALLOW_TIMEOUT_MS * 1000 % 1000000;
    FD_ZERO(&set);
    FD_SET(fd, &set);

    sem_post(&event_read_rdy_sem);

    while (1)
    {
         while (read(fd, &ev, sizeof(struct input_event)) != -1)
            {
                /* code */
            }
        sem_wait(&key_singal_sem);
        key_event_detect_flag = 0;

        while (!key_event_detect_flag)
        {

            timeout.tv_sec = MAX_ALLOW_TIMEOUT_MS / 1000;
            timeout.tv_usec = MAX_ALLOW_TIMEOUT_MS * 1000 % 1000000;
            FD_ZERO(&set);
            FD_SET(fd, &set);
            ret = select(fd + 1, &set, NULL, NULL, &timeout);
            if (ret == -1) // error
            {
                perror("select");
                error_flag = 1;
                sem_post(&key_detect_sem);
                break;
            }
            else if (ret == 0) // timeout
            {
                timeout_times++;
                timeout_flag = 1;
                sem_post(&key_detect_sem);
                if (timeout_times >= MAX_ALLOW_TIMEOUT_TIMES)
                {
                    break;
                }
                break;
            }
            else if (ret == 1)
            {
                if FD_ISSET (fd, &set)
                {
                    ret = read(fd, &ev, sizeof(struct input_event));
                    if (ret == -1)
                    {
                        perror("read");
                        error_flag = 1;
                        sem_post(&key_detect_sem);
                        break;
                    }
                    else if (ret != sizeof(struct input_event))
                    {
                        fprintf(stderr, "read: unexpected size %d\n", ret);
                        break;
                    }
                    else
                    {
                        timestamp_overtime_detect = get_timestamp_us();
                        if ((trigger_test_flag && ((timestamp_overtime_detect - timestamp_key_trigger_signal) > (MAX_ALLOW_TIMEOUT_MS * 1000))) || (release_test_flag && ((timestamp_overtime_detect - timestamp_key_release_signal) > (MAX_ALLOW_TIMEOUT_MS * 1000))))
                        {
                            timeout_times++;
                            timeout_flag = 1;
                            sem_post(&key_detect_sem);
                            if (timeout_times >= MAX_ALLOW_TIMEOUT_TIMES)
                            {
                                break;
                            }
                            break;
                        }
                        if (ev.type == EV_KEY)
                        {
                            if (ev.value == 1 && ev.code == ev_arg->event_value && trigger_test_flag)
                            {
                                timestamp_key_trigger_detect = ev.time.tv_sec * 1000000 + ev.time.tv_usec;
                                key_event_detect_flag = 1;
                                sem_post(&key_detect_sem);
                                break;
                            }
                            else if (ev.value == 0 && ev.code == ev_arg->event_value && release_test_flag)
                            {
                                timestamp_key_release_detect = ev.time.tv_sec * 1000000 + ev.time.tv_usec;
                                key_event_detect_flag = 1;
                                sem_post(&key_detect_sem);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int kbLatencyTest(struct kb_lat_test_params *params)
{
    // 初始化信号量
    sem_init(&event_read_rdy_sem, 0, 0);
    sem_init(&key_singal_sem, 0, 0);
    sem_init(&key_detect_sem, 0, 0);

    // 创建事件读取线程
    pthread_t tid;
    struct event_arg ev_arg;
    ev_arg.event_num = params->ev_num;
    ev_arg.event_value = params->ev_value;
    pthread_create(&tid, NULL, (void *)kbEventReadThread, &ev_arg);
    sleep(1);
    int res = sem_trywait(&event_read_rdy_sem);
    if (res != 0)
    {
        // 将进程结束
        pthread_cancel(tid);
        printf("事件读取线程创建失败\n");
        return -1;
    }

    printf("【测试编号】\t【触发延迟】\t【抬起延迟】\n");
    // 设置初始状态
    gpiod_line_set_value(params->keyboard_trigger_line, GPIOD_LINE_KEYBOARD_TRIGGER_RELEASE_VAL);
    gpiod_line_set_value(params->tester_mode_sel_line, params->test_mode == 1 ? GPIOD_LINE_TESTER_MODE_SEL_RESISTIVE_LOAD_TOP_VAL : GPIOD_LINE_TESTER_MODE_SEL_SHORT_VAL);

    usleep(500000);
    // 开始测试
    for (int i = 0; i < params->number_of_test; i++)
    {
        // 等待按键触发
        trigger_test_flag = 1;
        timestamp_key_trigger_signal = get_timestamp_us();
        gpiod_line_set_value(params->keyboard_trigger_line, GPIOD_LINE_KEYBOARD_TRIGGER_HOLD_VAL);
        sem_post(&key_singal_sem);
        sem_wait(&key_detect_sem);
        if (timeout_flag)
        {
            timeout_flag = 0;
            trigger_timeout_flag = 1;
            if (timeout_times >= MAX_ALLOW_TIMEOUT_TIMES)
            {
                break;
            }
        }
        if (error_flag)
        {
            break;
        }
        trigger_test_flag = 0;

        usleep(params->time_of_hold * 1000); // 等待按键保持

        // 等待按键抬起
        release_test_flag = 1;
        timestamp_key_release_signal = get_timestamp_us();
        gpiod_line_set_value(params->keyboard_trigger_line, GPIOD_LINE_KEYBOARD_TRIGGER_RELEASE_VAL);
        sem_post(&key_singal_sem);
        sem_wait(&key_detect_sem);
        if (timeout_flag)
        {
            timeout_flag = 0;
            release_timeout_flag = 1;
            if (timeout_times >= MAX_ALLOW_TIMEOUT_TIMES)
            {
                break;
            }
        }
        if (error_flag)
        {
            break;
        }
        release_test_flag = 0;

        // 计算延迟
        key_trigger_latency = timestamp_key_trigger_detect - timestamp_key_trigger_signal;
        key_release_latency = timestamp_key_release_detect - timestamp_key_release_signal;
        if (trigger_timeout_flag)
        {
            key_trigger_latency = -1;
            trigger_timeout_flag = 0;
        }
        if (release_timeout_flag)
        {
            key_release_latency = -1;
            release_timeout_flag = 0;
        }
        trigger_test_result[i] = key_trigger_latency;
        release_test_result[i] = key_release_latency;
        actual_test_times++;
        printf("【%03d】\t\t%7ld us\t%7ld us\n", i + 1, key_trigger_latency, key_release_latency);

        // printf("【%03d】\t\t%7ld us\t%7ld us\n", i + 1, timestamp_key_trigger_detect, timestamp_key_trigger_signal);
        // printf("【%03d】\t\t%7ld us\t%7ld us\n", i + 1, timestamp_key_release_detect, timestamp_key_release_signal);

        usleep(params->time_between_each_test * 1000); // 等待下一次测试
    }

    // 结束线程
    pthread_cancel(tid);

    // 释放信号量
    sem_destroy(&event_read_rdy_sem);
    sem_destroy(&key_singal_sem);
    sem_destroy(&key_detect_sem);

    return 0;
}
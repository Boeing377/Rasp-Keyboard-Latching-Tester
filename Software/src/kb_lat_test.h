#ifndef __KB_LAT_TEST_H__
#define __KB_LAT_TEST_H__

#include <gpiod.h>
#include <pthread.h>
#include <semaphore.h>

#define GPIOD_CHIPNAME "gpiochip4"
#define GPIOD_LINE_KEYBOARD_TRIGGER 2                       // 键盘触发信号线
#define GPIOD_LINE_KEYBOARD_TRIGGER_INIT_VAL 0              // 键盘触发信号线初始值
#define GPIOD_LINE_KEYBOARD_TRIGGER_HOLD_VAL 1              // 键盘触发信号线保持值
#define GPIOD_LINE_KEYBOARD_TRIGGER_RELEASE_VAL 0           // 键盘触发信号线释放值
#define GPIOD_LINE_TESTER_MODE_SEL 3                        // 测试器模式选择信号线
#define GPIOD_LINE_TESTER_MODE_SEL_INIT_VAL 0               // 测试器模式选择信号线初始值
#define GPIOD_LINE_TESTER_MODE_SEL_SHORT_VAL 0              // 测试器模式选择信号线短接触底测试值
#define GPIOD_LINE_TESTER_MODE_SEL_RESISTIVE_LOAD_TOP_VAL 1 // 测试器模式选择信号线电阻负载顶部触发测试值

#define MAX_ALLOW_TIMEOUT_TIMES 3 // 最大允许超时次数
#define MAX_ALLOW_TIMEOUT_MS 5000 // 最大允许超时时间 ms
#define MAX_ALLOW_ERROR_TIMES 3   // 最大允许错误次数

struct kb_lat_test_params
{
    int number_of_test;                       // 测试次数
    int time_of_hold;                         // 按键保持时长 ms
    int time_between_each_test;               // 测试间隔 ms
    int test_mode;                            // 测试模式 0: 短接触底测试 1: 电阻负载顶部触发测试 2: 电阻负载底部触发测试
    struct gpiod_line *keyboard_trigger_line; // 键盘触发信号线
    struct gpiod_line *tester_mode_sel_line;  // 测试器模式选择信号线

    int ev_num;   // 键盘事件编号
    int ev_value; // 键盘事件值
};

int kbLatencyTest(struct kb_lat_test_params *params);

#endif
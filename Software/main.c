#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/input.h>
#include <semaphore.h>
#include <gpiod.h>
#include <termios.h>
#include <signal.h>

#include "kb_ev_det.h"
#include "result_data_processing.h"
#include "kb_lat_test.h"

#define VERSION "1.0.0" // 软件版本号

#define DEF_NUM_OF_TEST 10            // 默认测试次数
#define DEF_TIME_OF_HOLD 10           // 默认按键时长 ms
#define DEF_TIME_BETWEEN_EACH_TEST 50 // 默认测试间隔 ms
#define DEF_TEST_MODE 0               // 默认测试模式

#define DEF_SAVE_FILE "kb_latency_test_result" // 默认存储文件名

// 软件运行参数
int windowsWidth = 80; // 窗口宽度

const char *chipname = GPIOD_CHIPNAME;
struct gpiod_chip *chip;                  // GPIO设备
struct gpiod_line *keyboard_trigger_line; // 键盘触发线
struct gpiod_line *tester_mode_sel_line;  // 测试器模式选择线
pthread_t adjust_tid;                     // 调整线程ID
char clean_stdin_buf[1024];               // 清空输入缓冲区

// 测试参数
int number_of_test = DEF_NUM_OF_TEST;                    // 测试次数
int time_of_hold = DEF_TIME_OF_HOLD;                     // 按键保持时长 ms
int time_between_each_test = DEF_TIME_BETWEEN_EACH_TEST; // 测试间隔 ms
int test_mode = 0;                                       // 测试模式 0: 短接触底测试 1: 电阻负载顶部触发测试 2: 电阻负载底部触发测试
int save_file = 0;                                       // 是否保存文件
char *filename = DEF_SAVE_FILE;                          // 存储文件名
int adjust_mode = 0;                                     // 调整模式 0: 不调整 1: 电阻调试模式

// 信号量
sem_t keyboardEventDetectSem; // 键盘事件检测信号量

/*
 * @brief: 键盘事件号检测线程
 * @param: arg: 传入参数
 * @return: 键盘事件号检测结果
 */
struct kb_ev_det_result_t keyboardEventDetectThread(void *arg)
{
    struct kb_ev_det_result_t kb_ev_res;
    sem_post(&keyboardEventDetectSem);
    kb_ev_res = kbEventDetect();
    return kb_ev_res;
}

/*
 * @brief: 键盘事件检测
 * @return: 键盘事件检测结果
 */
struct kb_ev_det_result_t keyboardEventDetect()
{
    struct kb_ev_det_result_t kb_ev;
    pthread_t tid;

    sem_init(&keyboardEventDetectSem, 0, 0);
    pthread_create(&tid, NULL, (void *)keyboardEventDetectThread, NULL);
    sem_wait(&keyboardEventDetectSem);
    gpiod_line_set_value(keyboard_trigger_line, GPIOD_LINE_KEYBOARD_TRIGGER_HOLD_VAL);

    pthread_join(tid, (void *)&kb_ev);
    gpiod_line_set_value(keyboard_trigger_line, GPIOD_LINE_KEYBOARD_TRIGGER_RELEASE_VAL);
    return kb_ev;
}

// 关闭控制台输入回显
void closeEcho()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// 打开控制台输入回显
void openEcho()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// 处理 Ctrl+C 信号
void handle_sigint(int sig)
{
    printf("\n捕获到信号 %d，正在恢复终端设置并退出...\n", sig);

    // 释放GPIO资源
    gpiod_line_set_value(keyboard_trigger_line, GPIOD_LINE_KEYBOARD_TRIGGER_RELEASE_VAL);
    gpiod_line_release(keyboard_trigger_line);
    gpiod_line_release(tester_mode_sel_line);

    // 强制关闭子线程
    pthread_cancel(adjust_tid);
    pthread_join(adjust_tid, NULL);

    // 恢复终端设置
    printf("按下回车键退出\n");
    fgets(clean_stdin_buf, 1024, stdin);
    openEcho();

    exit(0); // 退出程序
}

// 获取终端窗口宽度
int getWindowsWidth()
{
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return size.ws_col;
}

// 在调整模式下，调整GPIO输出
void adjustGpioOutput(void *arg)
{
    gpiod_line_set_value(tester_mode_sel_line, GPIOD_LINE_TESTER_MODE_SEL_RESISTIVE_LOAD_TOP_VAL);
    while (1)
    {
        gpiod_line_set_value(keyboard_trigger_line, GPIOD_LINE_KEYBOARD_TRIGGER_HOLD_VAL);
        usleep(500000);
        gpiod_line_set_value(keyboard_trigger_line, GPIOD_LINE_KEYBOARD_TRIGGER_RELEASE_VAL);
        usleep(500000);
    }
}

// 初始化显示
void init_show()
{
    for (int i = 0; i < windowsWidth; i++)
    {
        putchar('=');
    }
    putchar('\n');

    if (adjust_mode)
    {
        printf("【警告】: 电阻调试模式\n");
        printf("请调整电阻，当触发信号给出时，刚好能够触发键盘信号\n");
        printf("确保连接好测试器，按下回车键开始调试\n");
        printf("调试结束后，使用 Ctrl+C 退出\n");
    }
    else
    {
        printf("进行键盘延迟测试\n");
        printf("测试次数: %3d\t", number_of_test);
        printf("按键保持时长: %3d ms\t", time_of_hold);
        printf("测试间隔: %3d ms\n", time_between_each_test);
        printf("测试模式: %s\t", test_mode == 0 ? "短接触底测试" : (test_mode == 1 ? "电阻负载顶部触发测试" : "电阻负载底部触发测试"));
        printf("存储文件: %s\n", save_file ? filename : "否");
        printf("确保连接好测试器，按下回车键开始测试\n");
    }

    for (int i = 0; i < windowsWidth; i++)
    {
        putchar('=');
    }
    putchar('\n');
}

/*
 * @brief: 显示测试结果
 * @param: trigger_data: 触发延迟数据
 * @param: release_data: 抬起延迟数据
 * @param: size: 数据大小
 */
void show_result(int *trigger_data, int *release_data, int size)
{
    // 数据预处理
    trigger_data_size = data_pre_process(trigger_data, size);
    int trigger_max = findMax(trigger_data, trigger_data_size);
    int trigger_min = findMin(trigger_data, trigger_data_size);
    double trigger_avg = calculateAverage(trigger_data, trigger_data_size);
    double trigger_std_dev = calculateStandardDeviation(trigger_data, trigger_data_size);

    release_data_size = data_pre_process(release_data, size);
    int release_max = findMax(release_data, release_data_size);
    int release_min = findMin(release_data, release_data_size);
    double release_avg = calculateAverage(release_data, release_data_size);
    double release_std_dev = calculateStandardDeviation(release_data, release_data_size);

    for (int i = 0; i < windowsWidth; i++)
    {
        putchar('=');
    }
    printf("【测试结果】\n");
    printf("【项目】\t【触发延迟】\t 【抬起延迟】\n");
    printf("【最大值】: \t%7d us\t %7d us\n", trigger_max, release_max);
    printf("【最小值】: \t%7d us\t %7d us\n", trigger_min, release_min);
    printf("【平均值】: \t%7.2f us\t %7.2f us\n", trigger_avg, release_avg);
    printf("【标准差】: \t%7.2f us\t %7.2f us\n", trigger_std_dev, release_std_dev);
}

/*
 * @brief: GPIO初始化
 * @return: 初始化结果
 */
int gpio_init()
{
    chip = gpiod_chip_open_by_name(chipname);
    if (!chip)
    {
        perror("Open chip failed\n");
        return -1;
    }

    keyboard_trigger_line = gpiod_chip_get_line(chip, GPIOD_LINE_KEYBOARD_TRIGGER);
    if (!keyboard_trigger_line)
    {
        perror("Get line failed\n");
        return -1;
    }

    tester_mode_sel_line = gpiod_chip_get_line(chip, GPIOD_LINE_TESTER_MODE_SEL);
    if (!tester_mode_sel_line)
    {
        perror("Get line failed\n");
        return -1;
    }

    int ret = gpiod_line_request_output(keyboard_trigger_line, "keyboard_trigger", GPIOD_LINE_KEYBOARD_TRIGGER_INIT_VAL);
    if (ret < 0)
    {
        perror("Request line as output failed\n");
        return -1;
    }

    ret = gpiod_line_request_output(tester_mode_sel_line, "tester_mode_sel", GPIOD_LINE_TESTER_MODE_SEL_INIT_VAL);
    if (ret < 0)
    {
        perror("Request line as output failed\n");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int show_help = 0;
    int show_version = 0;
    int res;

    int opt;
    while ((opt = getopt(argc, argv, "hln:vb:t:s::a")) != -1)
    {
        switch (opt)
        {
        case 'h':
            show_help = 1;
            break;
        case 'l':
            test_mode = 1;
            break;
        case 'n':
            number_of_test = atoi(optarg);
            break;
        case 'v':
            show_version = 1;
            break;
        case 'b':
            time_between_each_test = atoi(optarg);
            break;
        case 't':
            time_of_hold = atoi(optarg);
            break;
        case 's': // 获取存储文件名，如没有，默认
            save_file = 1;
            if (optarg != NULL)
                filename = optarg;
            break;
        case 'a':
            adjust_mode = 1;
            break;
        default:
            // fprintf(stderr, "Usage: %s [-h] [-v] [-s] [-t timeout]\n", argv[0]);
            fprintf(stderr, "用法: %s [-h] [-v] [-n 测试次数] [-b 测试间隔] [-t 保持时间] [-s] [-a]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    windowsWidth = getWindowsWidth();

    if (show_help)
    {
        printf("用法: %s [选项]\n", argv[0]);
        printf("选项:\n");
        printf("  -h            显示帮助信息\n");
        printf("  -v            显示软件版本\n");
        printf("  -n <value>    测试次数\n");
        printf("  -t <value>    设置按键保持时长 ms\n");
        printf("  -b <value>    设置测试间隔 ms\n");
        printf("  -l            电阻负载顶部触发测试\n");
        printf("  -s [filename] 保存测试结果到文件\n");
        printf("  -a            电阻调试模式\n");
        return 0;
    }

    if (show_version)
    {
        printf("Version: %s\n", VERSION);
        return 0;
    }

    if (gpio_init() < 0)
    {
        return -1;
    }

    init_show();

    closeEcho();
    // 按回车键开始测试
    getchar();
    sleep(1);

    struct kb_ev_det_result_t kb_ev_res;
    kb_ev_res = keyboardEventDetect();
    printf("键盘事件位于: %d\n", kb_ev_res.event_num);

    sleep(1);

    if (adjust_mode)
    {
        signal(SIGINT, handle_sigint);
        pthread_create(&adjust_tid, NULL, (void *)adjustGpioOutput, NULL);
        kb_Event_Detect(kb_ev_res.event_num);
    }

    printf("测试开始\n");
    struct kb_lat_test_params params = {
        .number_of_test = number_of_test,
        .time_of_hold = time_of_hold,
        .time_between_each_test = time_between_each_test,
        .test_mode = test_mode,
        .keyboard_trigger_line = keyboard_trigger_line,
        .tester_mode_sel_line = tester_mode_sel_line,
        .ev_num = kb_ev_res.event_num,
        .ev_value = kb_ev_res.value};
    res = kbLatencyTest(&params);
    if (res < 0)
    {
        printf("测试失败\n");
    }
    else
    {
        printf("测试结束\n");
    }

    show_result(trigger_test_result, release_test_result, actual_test_times);

    if (save_file)
    {
        generateCSV(filename, trigger_test_result, trigger_data_size, release_test_result, release_data_size);
    }

    gpiod_line_set_value(keyboard_trigger_line, GPIOD_LINE_KEYBOARD_TRIGGER_RELEASE_VAL);
    gpiod_line_release(keyboard_trigger_line);
    gpiod_line_release(tester_mode_sel_line);

    printf("按下回车键退出\n");
    fgets(clean_stdin_buf, 1024, stdin);
    openEcho();

    return 0;
}
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <gpiod.h>

#define CHIP_NAME "gpiochip4"   // gpiochip4
#define OUTPUT_LINE_OFFSET 2     // 0
#define INPUT_LINE_OFFSET 3      // 1   

unsigned long get_timestamp_us()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

int main()
{
    struct gpiod_chip *chip;
    struct gpiod_line *output_line, *input_line;
    unsigned long timestamp0;
    unsigned long timestamp1;
    unsigned long timestamp2;
    unsigned long diff1, diff2;
    int ret;

    chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip)
    {
        perror("Open chip failed\n");
        return -1;
    }

    output_line = gpiod_chip_get_line(chip, OUTPUT_LINE_OFFSET);
    if (!output_line)
    {
        perror("Get output line failed\n");
        return -1;
    }

    input_line = gpiod_chip_get_line(chip, INPUT_LINE_OFFSET);
    if (!input_line)
    {
        perror("Get input line failed\n");
        return -1;
    }

    ret = gpiod_line_request_output(output_line, "output", 0);
    if (ret < 0)
    {
        perror("Request output failed\n");
        return -1;
    }

    ret = gpiod_line_request_input(input_line, "input");
    if (ret < 0)
    {
        perror("Request input failed\n");
        return -1;
    }

    for (int i = 0; i < 10; i++)
    {
        // Get timestamp before setting output high
        timestamp0 = get_timestamp_us();

        // Set output line to high
        ret = gpiod_line_set_value(output_line, 1);
        if (ret < 0)
        {
            perror("Set output line high failed\n");
            return -1;
        }

        // Get timestamp after setting output high
        timestamp1 = get_timestamp_us();

        // Wait for input line to go high
        while (gpiod_line_get_value(input_line) == 0);

        // Get timestamp when input goes high
        timestamp2 = get_timestamp_us();

        // Calculate the difference
        diff1 = timestamp1 - timestamp0;
        diff2 = timestamp2 - timestamp1;
        // Print the delay
        // printf("%d:timestamp0: %lu, timestamp1: %lu, timestamp2: %lu, diff1: %lu, diff2: %lu\n", i, timestamp0, timestamp1, timestamp2, diff1, diff2);
        printf("%d:diff2: %lu us\n", i, diff2);

        // Set output line to low
        ret = gpiod_line_set_value(output_line, 0);
        if (ret < 0)
        {
            perror("Set output line low failed\n");
            return -1;
        }

        // Small delay to ensure proper toggling
        usleep(1000);
    }

    gpiod_line_release(output_line);
    gpiod_line_release(input_line);
    gpiod_chip_close(chip);
    return 0;
}
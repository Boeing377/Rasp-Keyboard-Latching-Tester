#include <time.h>
#include <stdio.h>
#include <unistd.h>

unsigned long get_timestamp_us()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

int main()
{
    volatile unsigned long timestamp1;
    volatile unsigned long timestamp2;
    for (int i = 0; i < 10; i++)
    {
        timestamp1 = get_timestamp_us();
        timestamp2 = get_timestamp_us();
        printf("%d:timestamp1: %lu, timestamp2: %lu, diff: %lu\n", i,timestamp1, timestamp2, timestamp2 - timestamp1);
        sleep(1);
    }
    return 0;
}
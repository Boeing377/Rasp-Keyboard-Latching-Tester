#include "result_data_processing.h"
#include <math.h>
#include <stdio.h>

int trigger_test_result[MAX_DATA_SIZE];
int release_test_result[MAX_DATA_SIZE];
int actual_test_times = 0;

int trigger_data_size;
int release_data_size;

int data_pre_process(int *data, int size)
{
    int count = 0;
    //remove all the data that is less than 0
    for (int i = 0; i < size; i++)
    {
        if (data[i] >= 0)
        {
            data[count++] = data[i];
        }
    }

    // 计算数据平均值和标准差，用于数据预处理，去除异常值
    double mean = calculateAverage(data, size);
    double std_dev = calculateStandardDeviation(data, size);
    int countdata = 0;
    for (int i = 0; i < count; i++)
    {
        if (fabs(data[i] - mean) < 3 * std_dev)
        {
            data[countdata++] = data[i];
        }
    }

    // // 移除所有的超时数据
    // for (int i = 0; i < count; i++)
    // {
    //     if (data[i] <= -1)
    //     {
    //         for (int j = i; j < count - 1; j++)
    //         {
    //             data[j] = data[j + 1];
    //         }
    //         count--;
    //     }
    // }

    return countdata;
}

int findMax(const int *data, size_t size)
{
    int max = data[0];
    for (size_t i = 1; i < size; i++)
    {
        if (data[i] > max)
        {
            max = data[i];
        }
    }
    return max;
}

int findMin(const int *data, size_t size)
{
    int min = data[0];
    for (size_t i = 1; i < size; i++)
    {
        if (data[i] < min)
        {
            min = data[i];
        }
    }
    return min;
}

double calculateAverage(const int *data, size_t size)
{
    double sum = 0;
    for (size_t i = 0; i < size; i++)
    {
        sum += data[i];
    }
    return sum / size;
}

double calculateStandardDeviation(const int *data, size_t size)
{
    double mean = calculateAverage(data, size);
    double sum = 0;
    for (size_t i = 0; i < size; i++)
    {
        sum += (data[i] - mean) * (data[i] - mean);
    }
    return sqrt(sum / size);
}

void generateCSV(const char *filename, const int *trigger_data, size_t trigger_data_size, const int *release_data, size_t release_data_size)
{
    int i = 0;
    char filename_csv[100];
    sprintf(filename_csv, "%s.csv", filename);
    FILE *file = fopen(filename_csv, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    fprintf(file, "测试次数,触发延迟,抬起延迟\n");

    if (trigger_data_size < release_data_size)
    {
        for (i = 0; i < trigger_data_size; i++)
        {
            fprintf(file, "%d,%d,%d\n", i, trigger_data[i], release_data[i]);
        }
        for (; i < release_data_size; i++)
        {
            fprintf(file, "%d,%d,%d\n", i, 0, release_data[i]);
        }
    }
    else
    {
        for (i = 0; i < release_data_size; i++)
        {
            fprintf(file, "%d,%d,%d\n", i, trigger_data[i], release_data[i]);
        }
        for (; i < trigger_data_size; i++)
        {
            fprintf(file, "%d,%d,%d\n", i, trigger_data[i], 0);
        }
    }

    fclose(file);
}
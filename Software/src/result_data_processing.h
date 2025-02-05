#ifndef RESULT_DATA_PROCESSING_H
#define RESULT_DATA_PROCESSING_H

#include <stddef.h>

#define MAX_DATA_SIZE 1000

extern int trigger_test_result[];
extern int release_test_result[];
extern int actual_test_times;

extern int trigger_data_size;
extern int release_data_size;

// Function prototypes for data processing
int data_pre_process(int *data, int size);
int findMax(const int *data, size_t size);
int findMin(const int *data, size_t size);
double calculateAverage(const int *data, size_t size);
double calculateStandardDeviation(const int *data, size_t size);
void generateCSV(const char *filename, const int *trigger_data, size_t trigger_data_size, const int *release_data, size_t release_data_size);

#endif // RESULT_DATA_PROCESSING_H

// Contributers: Finn Jakob

#ifndef METRICS_H
#define METRICS_H

#define DATA_SIZE 15

// Enum for accessing metrics data indices
typedef enum {
    KEY_PRESS_COUNTS = 0,
    KEY_PRESS_INTERVALS,
    ENTER_COUNTS,
    BACKSPACE_COUNTS,
    LEFT_CLICK_COUNTS,
    RIGHT_CLICK_COUNTS,
    METRICS_COUNT // Used to size arrays
} MetricsIndices;

// Data structure for metrics
typedef struct {
    float data[METRICS_COUNT][DATA_SIZE];
    int currentIndex;
} MetricsData;

// Declare metrics object
extern MetricsData metrics;

#endif // METRICS_H

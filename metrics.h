#ifndef METRICS_H
#define METRICS_H

#define DATA_SIZE 15

// Enumeration für die Metrik-Typen
typedef enum {
    KEY_PRESS_COUNTS,
    KEY_PRESS_INTERVALS,
    ENTER_COUNTS,
    BACKSPACE_COUNTS,
    LEFT_CLICK_COUNTS,
    RIGHT_CLICK_COUNTS,
    METRIC_COUNT // Anzahl der Metriken
} MetricType;

// Struktur für die Metrik-Daten
typedef struct
{
    float data[METRIC_COUNT][DATA_SIZE];
    int currentIndex;
} MetricsData;

// Globale Variable
extern MetricsData metrics;

#endif


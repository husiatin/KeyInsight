#include "writeyaml.h"
#include "writelogs.h"
#include "metrics.h"
#include "utils.h"

// Array to store keyboard-press-per-mouse-click calculations
#define HOUR_ARRAY_SIZE 4
float hourlyKeyPressPerClick[HOUR_ARRAY_SIZE] = {0}; // Stores up to 4 values
int hourArrayIndex = 0; // Tracks the current position in the array
bool isHourArrayFull = false; // Tracks if the array has been filled once

// Function to calculate the average of an array
float calculateAverage(float *data, int size) {
    float sum = 0.0f;
    int count = 0;

    for (int i = 0; i < size; i++) {
        if (data[i] > 0) {
            sum += data[i];
            count++;
        }
    }

    return (count == 0) ? 0.0f : sum / count;
}

// Function to perform calculations and store results
void performCalculations() {
    // Calculate averages
    float avgKeyPresses = calculateAverage(metrics.data[KEY_PRESS_COUNTS], DATA_SIZE);
    float avgKeyPressesInterval = calculateAverage(metrics.data[KEY_PRESS_INTERVALS], DATA_SIZE);
    float avgEnterPresses = calculateAverage(metrics.data[ENTER_COUNTS], DATA_SIZE);
    float avgBackspacePresses = calculateAverage(metrics.data[BACKSPACE_COUNTS], DATA_SIZE);
    float avgLeftClicks = calculateAverage(metrics.data[LEFT_CLICK_COUNTS], DATA_SIZE);
    float avgRightClicks = calculateAverage(metrics.data[RIGHT_CLICK_COUNTS], DATA_SIZE);

    // Keyboard presses per mouse click (15-minute calculation)
    float keyPressPerClick = calculateSum(metrics.data[KEY_PRESS_COUNTS], DATA_SIZE) / 
                            (calculateSum(metrics.data[RIGHT_CLICK_COUNTS], DATA_SIZE) + calculateSum(metrics.data[LEFT_CLICK_COUNTS], DATA_SIZE));

    // Update the 4-slot hourly array
    hourlyKeyPressPerClick[hourArrayIndex] = keyPressPerClick;
    hourArrayIndex = (hourArrayIndex + 1) % HOUR_ARRAY_SIZE;

    // Mark array as full once all slots have been used
    if (hourArrayIndex == 0) {
        isHourArrayFull = true;
    }

    // Calculate one-hour average from the array
    float oneHourKeyPressPerClick = calculateAverage(hourlyKeyPressPerClick, isHourArrayFull ? HOUR_ARRAY_SIZE : hourArrayIndex);

    // Total mouse clicks and difference
    float totalClicks = avgLeftClicks + avgRightClicks;
    float clickDifference = avgLeftClicks - avgRightClicks;

    // Write results to .log file
    writeResultsToLog(avgKeyPresses, avgKeyPressesInterval, avgEnterPresses, avgBackspacePresses, totalClicks, avgLeftClicks, avgRightClicks, clickDifference, keyPressPerClick, oneHourKeyPressPerClick);

    // Write results to YAML
    writeResultsToYaml(avgKeyPresses, avgKeyPressesInterval, avgEnterPresses, avgBackspacePresses, totalClicks, avgLeftClicks, avgRightClicks, clickDifference, keyPressPerClick, oneHourKeyPressPerClick);
}

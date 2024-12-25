// Contributers: Finn Jakob, Janine Patulada, Kyrylo Zhulai

#include <stdio.h>
#include <windows.h>
#include "metrics.h"
#include "writeyaml.c"
#include "writelogs.c"
#include "utils.h"

static float calculateSum(float *data, int size) {
    float sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

// Function to perform calculations and store results
void performCalculations()
{
    // Calculate averages
    float avgKeyPresses = calculateAverage(metrics.data[KEY_PRESS_COUNTS], DATA_SIZE);
    float avgKeyPressesInterval = calculateAverage(metrics.data[KEY_PRESS_INTERVALS], DATA_SIZE);
    float avgEnterPresses = calculateAverage(metrics.data[ENTER_COUNTS], DATA_SIZE);
    float avgBackspacePresses = calculateAverage(metrics.data[BACKSPACE_COUNTS], DATA_SIZE);
    float avgLeftClicks = calculateAverage(metrics.data[LEFT_CLICK_COUNTS], DATA_SIZE); // pflichtenheft: nicht durchschnitt sondern anz an klicks ??
    float avgRightClicks = calculateAverage(metrics.data[RIGHT_CLICK_COUNTS], DATA_SIZE);

    //--new-- Funktion zur Berechnung von Tastenanschlägen pro Mausklick achtung im pflichtenheft steht pro stunde 
    float keyPressPerClick = calculateSum(metrics.data[KEY_PRESS_COUNTS], DATA_SIZE) / ( calculateSum(metrics.data[RIGHT_CLICK_COUNTS], DATA_SIZE) + calculateSum(metrics.data[LEFT_CLICK_COUNTS], DATA_SIZE) );



    // Total mouse clicks and difference
    float totalClicks = avgLeftClicks + avgRightClicks;
    float clickDifference = avgLeftClicks - avgRightClicks;

    // Write results to .log file
    writeResultsToLog(avgKeyPresses, avgKeyPressesInterval, avgEnterPresses, avgBackspacePresses, totalClicks, avgLeftClicks, avgRightClicks, clickDifference, keyPressPerClick);

    // Write results to YAML
    writeResultsToYaml(avgKeyPresses, avgKeyPressesInterval, avgEnterPresses, avgBackspacePresses, totalClicks, avgLeftClicks, avgRightClicks, clickDifference, keyPressPerClick);
}


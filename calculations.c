
#include "writeyaml.h"
#include "writelogs.h"
#include "metrics.h"
#include "utils.h"




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


    // Total mouse clicks and difference
    float totalClicks = avgLeftClicks + avgRightClicks;
    float clickDifference = avgLeftClicks - avgRightClicks;

    // Write results to .log file
    writeResultsToLog(avgKeyPresses, avgKeyPressesInterval, avgEnterPresses, avgBackspacePresses, totalClicks, avgLeftClicks, avgRightClicks, clickDifference);

    // Write results to YAML
    writeResultsToYaml(avgKeyPresses, avgKeyPressesInterval, avgEnterPresses, avgBackspacePresses, totalClicks, avgLeftClicks, avgRightClicks, clickDifference);
}

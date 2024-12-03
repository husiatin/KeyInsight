#include <stdio.h>
#include <windows.h>

// Function to write averages to a .log file
void writeResultsToLog(float avgKeyPresses, float avgKeyPressesInterval, float avgEnterPresses, float avgBackspacePresses, float avgClicks, float avgLeftClicks, float avgRightClicks, float clickDifference)
{
    SYSTEMTIME lt;

    GetLocalTime(&lt);

    FILE *logFile = fopen("metrics.log", "a"); // Open the log file in append mode will create the file if it does not exist
    if (logFile == NULL)
    {
        printf("Error opening log file.\n");
        return;
    }

    // Write data to the log file
    fprintf(logFile, "----- %02d.%02d.%02d %02d:%02d:%02d -----\n", lt.wDay, lt.wMonth, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
    fprintf(logFile, "Average keypress intervals in seconds within 15 minutes: %.2fs\n", avgKeyPressesInterval);
    fprintf(logFile, "Average keypresses within 15 minutes: %.2f\n", avgKeyPresses);
    fprintf(logFile, "Average ENTER presses within 15 minutes: %.2f\n", avgEnterPresses);
    fprintf(logFile, "Average BACKSPACE presses within 15 minutes: %.2f\n", avgBackspacePresses);
    fprintf(logFile, "Average mouse clicks within 15 minutes: %.2f\n", avgClicks);
    fprintf(logFile, "Average left mouse clicks within 15 minutes: %.2f\n", avgLeftClicks);
    fprintf(logFile, "Average right mouse clicks within 15 minutes: %.2f\n", avgRightClicks);
    fprintf(logFile, "Difference between left and right clicks within 15 minutes: %.2f\n", clickDifference);
    fprintf(logFile, "-------------------\n");

    fclose(logFile); // Close the log file
    printf("Results written to metrics.log\n");
}
#include <stdio.h>
#include <windows.h>

// Function to write averages to a .log file
void writeResultsToLog(float avgKeyPresses, float avgKeyPressesInterval, float avgEnterPresses, float avgBackspacePresses, float totalClicks, float avgLeftClicks, float avgRightClicks, float clickDifference, float keyPressPerClick) {
    SYSTEMTIME lt;

    GetLocalTime(&lt);

    FILE *logFile = fopen("metrics.log", "a"); // Open the log file in append mode will create the file if it does not exist
    if (logFile == NULL) {
        printf("Error opening log file.\n");
        return;
    }

    // Write data to the log file
    fprintf(logFile, "----- %02d.%02d.%02d %02d:%02d:%02d -----\n", lt.wDay, lt.wMonth, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
    fprintf(logFile, "Average keypresses per minute: %.2f\n", avgKeyPresses);
    fprintf(logFile, "Average keypress-intervals per minute: %.2f\n", avgKeyPressesInterval);
    fprintf(logFile, "Average ENTER presses per minute: %.2f\n", avgEnterPresses);
    fprintf(logFile, "Average BACKSPACE presses per minute: %.2f\n", avgBackspacePresses);
    fprintf(logFile, "Total click count per minute: %.2f\n", totalClicks);
    fprintf(logFile, "Average left mouse-button clicks per minute: %.2f\n", avgLeftClicks);
    fprintf(logFile, "Average right mouse-button clicks per minute: %.2f\n", avgRightClicks);
    fprintf(logFile, "Difference between left and right clicks: %.2f\n", clickDifference);
    fprintf(logFile, "Average keypresses per mouseclick in one hour: %.2f\n", keyPressPerClick);
    fprintf(logFile, "-------------------\n");

    fclose(logFile); // Close the log file
    printf("Results written to metrics.log\n");
}

#include <stdio.h>
#include <windows.h>

// Function to write averages to a .log file
void writeResultsToLog(float avgKeyPresses, float avgEnterPresses, float avgBackspacePresses, float avgClicks, float clickDifference, float keyPressPerClick) {
    SYSTEMTIME st;

    GetSystemTime(&st);

    FILE *logFile = fopen("metrics.log", "a"); // Open the log file in append mode will create the file if it does not exist
    if (logFile == NULL) {
        printf("Error opening log file.\n");
        return;
    }

    // Write data to the log file
    fprintf(logFile, "----- %02d.%02d.%02d %02d:%02d:%02d -----\n", st.wDayOfWeek, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
    fprintf(logFile, "Average keypresses per minute: %.2f\n", avgKeyPresses);
    fprintf(logFile, "Average ENTER presses per minute: %.2f\n", avgEnterPresses);
    fprintf(logFile, "Average BACKSPACE presses per minute: %.2f\n", avgBackspacePresses);
    fprintf(logFile, "Average mouse clicks per minute: %.2f\n", avgClicks);
    fprintf(logFile, "Difference between left and right clicks: %.2f\n", clickDifference);
    fprintf(logFile, "Average keypresses per mouseclick in one hour: %.2f\n", keyPressPerClick);
    fprintf(logFile, "-------------------\n");

    fclose(logFile); // Close the log file
    printf("Results written to metrics.log\n");
}

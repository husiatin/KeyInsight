#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <process.h>
#include <stdlib.h>

// Constants
#define DATA_SIZE 15
#define ONE_MINUTE_MS 60000
#define FIFTEEN_MINUTES_MS 900000

// Data structure for metrics
typedef struct
{
    float keyPressIntervals[DATA_SIZE];
    float enterCounts[DATA_SIZE];
    float backspaceCounts[DATA_SIZE];
    float leftClickCounts[DATA_SIZE];
    float rightClickCounts[DATA_SIZE];
    int currentIndex;
} MetricsData;

// Global variables
MetricsData metrics = {0}; // Main buffer
bool running = true;
float keyPressTimes[1000] = {0}; // Buffer for keypress intervals
int keyPressIndex = 0;
bool isBufferFull = false; // Tracks if the buffer has been fully cycled once

// Function to calculate the average of an array
float calculateAverage(float *data, int size)
{
    float sum = 0.0;
    int count = 0;

    for (int i = 0; i < size; i++)
    {
        if (data[i] > 0)
        {
            sum += data[i];
            count++;
        }
    }

    return (count == 0) ? 0.0 : sum / count;
}

// Function to write results to YAML file
void writeResultsToYaml(float avgKeyPresses, float avgEnterPresses, float avgBackspacePresses, float avgClicks, float clickDifference)
{
    FILE *file = fopen("metrics.yaml", "w");
    if (file == NULL)
    {
        printf("Error opening file for writing.\n");
        return;
    }

    // Write data in YAML format
    fprintf(file, "metrics:\n");
    fprintf(file, "  average_keypresses_per_minute: %.2f\n", avgKeyPresses);
    fprintf(file, "  average_enter_presses_per_minute: %.2f\n", avgEnterPresses);
    fprintf(file, "  average_backspace_presses_per_minute: %.2f\n", avgBackspacePresses);
    fprintf(file, "  average_mouse_clicks_per_minute: %.2f\n", avgClicks);
    fprintf(file, "  click_difference_per_minute: %.2f\n", clickDifference);

    fclose(file);
    printf("Results written to metrics.yaml\n");
}

// Function to perform calculations and store results
void performCalculations()
{
    // Calculate averages
    float avgKeyPresses = calculateAverage(metrics.keyPressIntervals, DATA_SIZE);
    float avgEnterPresses = calculateAverage(metrics.enterCounts, DATA_SIZE);
    float avgBackspacePresses = calculateAverage(metrics.backspaceCounts, DATA_SIZE);
    float avgLeftClicks = calculateAverage(metrics.leftClickCounts, DATA_SIZE);
    float avgRightClicks = calculateAverage(metrics.rightClickCounts, DATA_SIZE);

    // Total mouse clicks and difference
    float totalClicks = avgLeftClicks + avgRightClicks;
    float clickDifference = avgLeftClicks - avgRightClicks;

    // Write results to YAML
    writeResultsToYaml(avgKeyPresses, avgEnterPresses, avgBackspacePresses, totalClicks, clickDifference);
}

// Logging thread to handle data storage for every minute and calculations every 15 minutes
void LogThread(void *param)
{
    DWORD lastLogTime = GetTickCount();
    DWORD lastCalculationTime = lastLogTime;

    float leftClickCount = 0.0;
    float rightClickCount = 0.0;

    while (running)
    {
        DWORD currentTime = GetTickCount();

        // Check if a minute has passed
        if (currentTime - lastLogTime >= ONE_MINUTE_MS)
        {
            // Calculate average of keypress intervals
            metrics.keyPressIntervals[metrics.currentIndex] = calculateAverage(keyPressTimes, keyPressIndex);

            // Store data in the current index of metrics buffer
            metrics.enterCounts[metrics.currentIndex] = metrics.enterCounts[metrics.currentIndex];
            metrics.backspaceCounts[metrics.currentIndex] = metrics.backspaceCounts[metrics.currentIndex];
            metrics.leftClickCounts[metrics.currentIndex] = leftClickCount;
            metrics.rightClickCounts[metrics.currentIndex] = rightClickCount;

            // Reset temporary counters
            keyPressIndex = 0;
            leftClickCount = 0.0;
            rightClickCount = 0.0;

            // Update the cyclic buffer index
            metrics.currentIndex = (metrics.currentIndex + 1) % DATA_SIZE;

            // Mark buffer as full after one complete cycle
            if (metrics.currentIndex == 0)
            {
                isBufferFull = true;
            }

            // Reset last log time
            lastLogTime = currentTime;
        }

        // Perform calculations every 15 minutes if buffer is full
        if (currentTime - lastCalculationTime >= FIFTEEN_MINUTES_MS && isBufferFull)
        {
            performCalculations();
            lastCalculationTime = currentTime;
        }

        Sleep(100); // Reduce CPU usage
    }
    _endthread();
}

// Keyboard hook to track keypress events
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    static DWORD lastKeyPressTime = 0;

    if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
    {
        KBDLLHOOKSTRUCT *pKeyInfo = (KBDLLHOOKSTRUCT *)lParam;
        DWORD currentTime = GetTickCount();

        // Record interval times between keypresses
        if (lastKeyPressTime > 0)
        {
            float interval = (float)(currentTime - lastKeyPressTime);
            keyPressTimes[keyPressIndex++] = interval;
            lastKeyPressTime = currentTime;
        }
        else
        {
            lastKeyPressTime = currentTime;
        }

        // Count specific keys
        if (pKeyInfo->vkCode == VK_RETURN)
        {
            metrics.enterCounts[metrics.currentIndex]++;
        }
        else if (pKeyInfo->vkCode == VK_BACK)
        {
            metrics.backspaceCounts[metrics.currentIndex]++;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Mouse hook to track button clicks
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        if (wParam == WM_LBUTTONDOWN)
        {
            metrics.leftClickCounts[metrics.currentIndex]++;
        }
        else if (wParam == WM_RBUTTONDOWN)
        {
            metrics.rightClickCounts[metrics.currentIndex]++;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{
    // Start the logging thread
    _beginthread(LogThread, 0, NULL);

    // Set up hooks
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

    if (!keyboardHook || !mouseHook)
    {
        printf("Failed to set hooks.\n");
        running = false;
        return 1;
    }

    // Keep the program running
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up when program ends
    UnhookWindowsHookEx(keyboardHook);
    UnhookWindowsHookEx(mouseHook);
    running = false;

    return 0;
}

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <process.h>
#include <stdlib.h>
#include "writelogs.c"
#include "writeyaml.h"


// Constants
#define DATA_SIZE 15
#define MINUTE_MS 60000
#define QUARTERH_MS 900000

// new 2d Data structure for metrics

typedef enum {
    KEY_PRESS_INTERVALS,
    KEY_PRESS_COUNTS,
    ENTER_COUNTS,
    BACKSPACE_COUNTS,
    LEFT_CLICK_COUNTS,
    RIGHT_CLICK_COUNTS,
    METRIC_COUNT // Anzahl der Metriken
} MetricType;

typedef struct
{
    float data[METRIC_COUNT][DATA_SIZE]; // Metrik-Daten
    int currentIndex;
} MetricsData;

// Global variables
MetricsData metrics = {0}; // Main buffer
bool running = true;
float keyPressTimes[1000] = {0}; // Buffer for keypress intervals
int keyPressIndex = 0;
bool isBufferFull = false; // Tracks if the buffer has been fully cycled once

// last code did not store data correctly temporary counters are necessary
float tempKeyPressCounts = 0;
float tempEnterCounts = 0;
float tempBackspaceCounts = 0;
float tempLeftClickCounts = 0;
float tempRightClickCounts = 0;

// Function to calculate the average of an array
float calculateAverage(float *data, int size)
{
    float sum = 0.0f;

    for (int i = 0; i < size; i++)
    {
        if (data[i] > 0)
        {
            sum += data[i];
        }
    }
    return (size == 0) ? 0.0f : sum / (float)size;
}

// Logging thread to handle data storage for every minute and calculations every 15 minutes
void LogThread(void *param)
{
    DWORD lastLogTime = GetTickCount();
    DWORD lastCalculationTime = lastLogTime;

    while (running)
    {
        DWORD currentTime = GetTickCount();

        // Check if a minute has passed
        if (currentTime - lastLogTime >= MINUTE_MS)
        {
            // Calculate average of keypress intervals
            metrics.data[KEY_PRESS_INTERVALS][metrics.currentIndex] = calculateAverage(keyPressTimes, keyPressIndex);

            // -- following code does nothing
            /*
            metrics.keyPressCounts[metrics.currentIndex];
            metrics.enterCounts[metrics.currentIndex];
            metrics.backspaceCounts[metrics.currentIndex];;
            metrics.leftClickCounts[metrics.currentIndex];
            metrics.rightClickCounts[metrics.currentIndex];
			*/
            
            // new code: Store data in the current index of metrics buffer 
        	metrics.data[KEY_PRESS_COUNTS][metrics.currentIndex] = tempKeyPressCounts;
    		metrics.data[ENTER_COUNTS][metrics.currentIndex] = tempEnterCounts;
   		metrics.data[BACKSPACE_COUNTS][metrics.currentIndex] = tempBackspaceCounts;
    		metrics.data[LEFT_CLICK_COUNTS][metrics.currentIndex] = tempLeftClickCounts;
    		metrics.data[RIGHT_CLICK_COUNTS][metrics.currentIndex] = tempRightClickCounts;

    		// Reset temporary counters
    		tempKeyPressCounts = 0;
    		tempEnterCounts = 0;
    		tempBackspaceCounts = 0;
    		tempLeftClickCounts = 0;
    		tempRightClickCounts = 0;

            keyPressIndex = 0;

            // Update array with FIFO principle
            metrics.currentIndex = (metrics.currentIndex + 1) % DATA_SIZE;

            // Mark buffer as full after one complete cycle
            if (metrics.currentIndex == 0)
            {
                isBufferFull = true;
            }

            // Reset last log time
            lastLogTime = currentTime;
        }

        // Reset array after 1 hour 
        if (currentTime - lastCalculationTime >= QUARTERH_MS && isBufferFull)
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

       tempKeyPressCounts++;

        // Record interval times between keypresses
        if (lastKeyPressTime > 0)
        {
            float interval = (float)(currentTime - lastKeyPressTime) / 1000;
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
            tempEnterCounts++;
        }
        else if (pKeyInfo->vkCode == VK_BACK)
        {
            tempBackspaceCounts++;
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
            tempLeftClickCounts++;
        }
        else if (wParam == WM_RBUTTONDOWN)
        {
            tempRightClickCounts++;
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
(

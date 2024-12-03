#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <process.h>
#include <stdlib.h>
#include "writelogs.c"
#include "writeyaml.c"
#include "calculations.c"

// Constants
#define DATA_SIZE 15
#define MINUTE_MS 60000
#define QUARTERH_MS 900000

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
    float data[METRIC_COUNT][DATA_SIZE]; 
    int currentIndex;
} MetricsData;

// Global variables
MetricsData metrics = {0}; // Main buffer
bool running = true;
float keyPressTimes[1000] = {0}; // Buffer for keypress intervals
int keyPressIndex = 0;
bool isBufferFull = false; // Tracks if the buffer has been fully cycled once
HANDLE mutex; // Mutex init for thread handling

// last code did not store data correctly temporary counters are necessary
float tempKeyPressCounts = 0;
float tempEnterCounts = 0;
float tempBackspaceCounts = 0;
float tempLeftClickCounts = 0;
float tempRightClickCounts = 0;

// -new- helper fuction sum of an array for new calc TotalPressPerClick
float calculateSum(float *data, int size)
{
    float sum = 0.0f;

    for (int i = 0; i < size; i++)
    {
        if (data[i] > 0)
        {
            sum += data[i];
        }
    }
    return sum;
}

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

// Function to prompt user for data collection duration
void setCollectionDuration() {
    printf("Enter data collection duration in minutes: ");
    int duration;
    scanf("%d", &duration);
    if (duration > 0) {
        collectionDurationMinutes = duration;
        printf("Data collection duration set to %d minutes.\n", collectionDurationMinutes);
    } else {
        printf("Invalid duration. Keeping the previous value: %d minutes.\n", collectionDurationMinutes);
    }
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
            WaitForSingleObject(mutex, INFINITE); 
            
            // Calculate average of keypress intervals
            metrics.data[KEY_PRESS_INTERVALS][metrics.currentIndex] = calculateAverage(keyPressTimes, keyPressIndex);
            
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

            ReleaseMutex(mutex); 
        }

        // send data to calculation after every 15min
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
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static DWORD lastKeyPressTime = 0;
    static bool ctrlPressed = false;

    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyInfo = (KBDLLHOOKSTRUCT *)lParam;

        WaitForSingleObject(mutex, INFINITE); // Lock the mutex for the entire shared resource block

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (pKeyInfo->vkCode == VK_CONTROL) {
                ctrlPressed = true;
            } else if (ctrlPressed && pKeyInfo->vkCode == 'K') {
                ReleaseMutex(mutex); // Unlock before opening the dialog
                setCollectionDuration(); // Trigger user prompt
                WaitForSingleObject(mutex, INFINITE); // Re-lock after dialog
            }

            // Track key press intervals
            if (lastKeyPressTime > 0) {
                DWORD currentTime = GetTickCount();
                float interval = (float)(currentTime - lastKeyPressTime) / 1000;
                keyPressTimes[keyPressIndex++] = interval;
                lastKeyPressTime = currentTime;
            } else {
                lastKeyPressTime = GetTickCount();
            }

            tempKeyPressCounts++; // Increment total keypress count
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (pKeyInfo->vkCode == VK_CONTROL) {
                ctrlPressed = false;
            }
        }

        // Count specific keys
        if (pKeyInfo->vkCode == VK_RETURN) {
            tempEnterCounts++; // Increment Enter key count
        } else if (pKeyInfo->vkCode == VK_BACK) {
            tempBackspaceCounts++; // Increment Backspace key count
        }

        ReleaseMutex(mutex); // Release the mutex after all shared resource updates
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Mouse hook to track button clicks
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        WaitForSingleObject(mutex, INFINITE); // Lock the mutex for shared resource updates

        if (wParam == WM_LBUTTONDOWN) {
            tempLeftClickCounts++; // Increment left click count
        } else if (wParam == WM_RBUTTONDOWN) {
            tempRightClickCounts++; // Increment right click count
        }

        ReleaseMutex(mutex); // Release the mutex
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{
    // Initialize mutex
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) {
        printf("Failed to create mutex.\n");
        return 1;
    }

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


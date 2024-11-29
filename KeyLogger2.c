#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <process.h> 
#include <stdlib.h>
#include "backgroundexecution.c"

// Constants for simpler code maintenance
#define DATA_SIZE 15
#define ONE_MINUTE_MS 60000

// Data object for storing data in arrays
typedef struct {
    float keyPressIntervals[DATA_SIZE];
    float enterCounts[DATA_SIZE];
    float backspaceCounts[DATA_SIZE];
    float leftClickCounts[DATA_SIZE];
    float rightClickCounts[DATA_SIZE];
    int currentIndex;
} MetricsData;

// Global variables
MetricsData metrics = {0}; 
bool running = true;
float keyPressTimes[1000] = {0}; // Buffer needed as helper for keypress intervals
int keyPressIndex = 0;

// Function to calculate the average of an array, used as helper function
float calculateAverage(float *data, int size) {
    float sum = 0.0;
    int count = 0;

    for (int i = 0; i < size; i++) {
        if (data[i] > 0) { 
            sum += data[i];
            count++;
        }
    }

    if (count == 0) return 0.0; 
    return sum / count;
}

// Logging thread to handle data storage for every minute
void LogThread(void *param) {
    DWORD lastLogTime = GetTickCount();

    float leftClickCount = 0.0;
    float rightClickCount = 0.0;

    while (running) {
        DWORD currentTime = GetTickCount();

        // Check if minute has passed
        if (currentTime - lastLogTime >= ONE_MINUTE_MS) {
            // Calculate avg of keypress intervals
            metrics.keyPressIntervals[metrics.currentIndex] = calculateAverage(keyPressTimes, keyPressIndex);

            // Store data
            metrics.enterCounts[metrics.currentIndex] = metrics.enterCounts[metrics.currentIndex];
            metrics.backspaceCounts[metrics.currentIndex] = metrics.backspaceCounts[metrics.currentIndex];
            metrics.leftClickCounts[metrics.currentIndex] = leftClickCount;
            metrics.rightClickCounts[metrics.currentIndex] = rightClickCount;

            // Reset counters
            keyPressIndex = 0;
            leftClickCount = 0.0;
            rightClickCount = 0.0;

            // Update indexes (FIFO principle)
            metrics.currentIndex = (metrics.currentIndex + 1) % DATA_SIZE;

            // Reset last log time
            lastLogTime = currentTime;
        }

        Sleep(100); // Reduce CPU usage
    }
    _endthread();
}

// Keyboard hook to track keypress events
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static DWORD lastKeyPressTime = 0;

    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *pKeyInfo = (KBDLLHOOKSTRUCT *)lParam;
        DWORD currentTime = GetTickCount();

        // Records interval times between keypresses
        if (lastKeyPressTime > 0) {
            float interval = (float)(currentTime - lastKeyPressTime);
            keyPressTimes[keyPressIndex++] = interval; // Update the global buffer
            lastKeyPressTime = currentTime;
        } else {
            lastKeyPressTime = currentTime;
        }

        // Counters for enter- and backspace-keys
        if (pKeyInfo->vkCode == VK_RETURN) {
            metrics.enterCounts[metrics.currentIndex]++;
        } else if (pKeyInfo->vkCode == VK_BACK) {
            metrics.backspaceCounts[metrics.currentIndex]++;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Mouse hook to track button clicks
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        if (wParam == WM_LBUTTONDOWN) {
            metrics.leftClickCounts[metrics.currentIndex]++;
        } else if (wParam == WM_RBUTTONDOWN) {
            metrics.rightClickCounts[metrics.currentIndex]++;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    // Start the program in the background
    RunInBackground();

    // Start the logging thread
    _beginthread(LogThread, 0, NULL);

    // Set up hooks
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

    if (!keyboardHook || !mouseHook) {
        printf("Failed to set hooks.\n");
        running = false;
        return 1;
    }

    // loop to keep the program running
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up when main thread closes
    UnhookWindowsHookEx(keyboardHook);
    UnhookWindowsHookEx(mouseHook);
    running = false;

    return 0;
}

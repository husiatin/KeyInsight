#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <process.h>
#include <stdlib.h>

// Constants
#define DATA_SIZE 15
#define MINUTE_MS 60000
#define QUARTERH_MS 900000 
#define HOUR_MS 3600000    

// Data structure for metrics
typedef struct {
    float keyPressIntervals[DATA_SIZE];
    float keyPressCounts[DATA_SIZE];
    float enterCounts[DATA_SIZE];
    float backspaceCounts[DATA_SIZE];
    float leftClickCounts[DATA_SIZE];
    float rightClickCounts[DATA_SIZE];
    int currentIndex;
} MetricsData;

// Global variables
MetricsData metrics = {0};
float keyPressTimes[1000] = {0}; // Buffer for keypress intervals
int keyPressIndex = 0;
int collectionDurationMinutes = 15; // Default collection duration (have to test it still)
bool running = true;
bool isBufferFull = false;
HANDLE mutex; // Mutex init for thread handling

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

// Function to reset the metrics buffer and reset indexes
void resetMetricsBuffer() {
    WaitForSingleObject(mutex, INFINITE); // Lock the mutex
    for (int i = 0; i < DATA_SIZE; i++) {
        metrics.keyPressIntervals[i] = 0.0;
        metrics.keyPressCounts[i] = 0.0;
        metrics.enterCounts[i] = 0.0;
        metrics.backspaceCounts[i] = 0.0;
        metrics.leftClickCounts[i] = 0.0;
        metrics.rightClickCounts[i] = 0.0;
    }
    metrics.currentIndex = 0; 
    isBufferFull = false;    
    ReleaseMutex(mutex);      // Mutex release standard func
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

// Function to write results to a YAML file
void writeResultsToYaml(float avgKeyPresses, float avgKeyPressesInterval, float avgEnterPresses, float avgBackspacePresses, float avgClicks, float avgLeftClicks, float avgRightClicks, float clickDifference) {
    FILE *file = fopen("metrics.yaml", "w");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    fprintf(file, "metrics:\n");
    fprintf(file, "  average_keypresses_interval_per_15_minutes: %.2f\n", avgKeyPressesInterval);
    fprintf(file, "  average_keypresses_per_15_minutes: %.2f\n", avgKeyPresses);
    fprintf(file, "  average_enter_presses_per_15_minutes: %.2f\n", avgEnterPresses);
    fprintf(file, "  average_backspace_presses_per_15_minutes: %.2f\n", avgBackspacePresses);
    fprintf(file, "  average_mouse_clicks_per_15_minutes: %.2f\n", avgClicks);
    fprintf(file, "  average_left_mouse_clicks_per_15_minutes: %.2f\n", avgLeftClicks);
    fprintf(file, "  average_right_mouse_clicks_per_15_minutes: %.2f\n", avgRightClicks);
    fprintf(file, "  click_difference_per_15_minutes: %.2f\n", clickDifference);

    fclose(file);
    printf("Results written to metrics.yaml\n");
}

// Function to perform calculations and write results
void performCalculations() {
    WaitForSingleObject(mutex, INFINITE); // Mutex Lock func (also standard)

    float avgKeyPresses = calculateAverage(metrics.keyPressCounts, DATA_SIZE);
    float avgKeyPressesInterval = calculateAverage(metrics.keyPressIntervals, DATA_SIZE);
    float avgEnterPresses = calculateAverage(metrics.enterCounts, DATA_SIZE);
    float avgBackspacePresses = calculateAverage(metrics.backspaceCounts, DATA_SIZE);
    float avgLeftClicks = calculateAverage(metrics.leftClickCounts, DATA_SIZE);
    float avgRightClicks = calculateAverage(metrics.rightClickCounts, DATA_SIZE);

    float totalClicks = avgLeftClicks + avgRightClicks;
    float clickDifference = avgLeftClicks - avgRightClicks;

    writeResultsToLog(avgKeyPresses, avgKeyPressesInterval, avgEnterPresses, avgBackspacePresses, totalClicks, avgLeftClicks, avgRightClicks, clickDifference);
    writeResultsToYaml(avgKeyPresses, avgKeyPressesInterval, avgEnterPresses, avgBackspacePresses, totalClicks, avgLeftClicks, avgRightClicks, clickDifference);

    ReleaseMutex(mutex); 
}

// Logging thread to handle data storage and calculations
void LogThread(void *param) {
    DWORD lastLogTime = GetTickCount();
    DWORD lastCalculationTime = lastLogTime;

    while (running) {
        DWORD currentTime = GetTickCount();

        // Check if a minute has passed
        if (currentTime - lastLogTime >= MINUTE_MS) {
            WaitForSingleObject(mutex, INFINITE); 

            metrics.keyPressIntervals[metrics.currentIndex] = calculateAverage(keyPressTimes, keyPressIndex);

            metrics.currentIndex = (metrics.currentIndex + 1) % DATA_SIZE;
            if (metrics.currentIndex == 0) {
                isBufferFull = true;
            }

            keyPressIndex = 0; 
            lastLogTime = currentTime;

            ReleaseMutex(mutex); 
        }

        // Perform calculations every 15 minutes
        if (currentTime - lastCalculationTime >= QUARTERH_MS && isBufferFull) {
            performCalculations();
            lastCalculationTime = currentTime;
        }

        // Clear buffer after one hour
        if (currentTime - lastCalculationTime >= HOUR_MS) {
            resetMetricsBuffer();
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

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (pKeyInfo->vkCode == VK_CONTROL) {
                ctrlPressed = true;
            } else if (ctrlPressed && pKeyInfo->vkCode == 'K') {
                setCollectionDuration(); // Trigger user prompt
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

            metrics.keyPressCounts[metrics.currentIndex]++;
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (pKeyInfo->vkCode == VK_CONTROL) {
                ctrlPressed = false;
            }
        }

        // Count specific keys
        if (pKeyInfo->vkCode == VK_RETURN) {
            metrics.enterCounts[metrics.currentIndex]++;
        } else if (pKeyInfo->vkCode == VK_BACK) {
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

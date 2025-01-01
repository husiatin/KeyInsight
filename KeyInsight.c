// Contributers: Andreas Lerch, Dustin Lojewski, Finn Jakob, Janine Patulada, Jannis Neuhaus, Kyrylo Zhulai

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <process.h>
#include <stdlib.h>
#include "calculations.c"
#include "metrics.h"
#include "utils.h"
#include "utils.c"  
#include <conio.h>
// #include "resource.h" - needed for Windows Message Box

// Constants
#define DATA_SIZE 15
#define MINUTE_MS 60000
#define QUARTERH_MS 900000

// Global variables
MetricsData metrics = {0}; // Main buffer
bool running = true;
float keyPressTimes[1000] = {0}; // Buffer for keypress intervals
int keyPressIndex = 0;
bool isBufferFull = false; // Tracks if the buffer has been fully cycled once
HANDLE mutex; // Mutex init for thread handling
int collectionDurationMinutes = 15;
DWORD endTime = 0;
HANDLE hConsole = NULL;

// last code did not store data correctly temporary counters are necessary
float tempKeyPressCounts = 0;
float tempEnterCounts = 0;
float tempBackspaceCounts = 0;
float tempLeftClickCounts = 0;
float tempRightClickCounts = 0;

//---
// Functions for starting and closing the terminal
void StartConsole() {
    if (hConsole != NULL) {
        // if console exists
        return;
    }

    // Terminal creation
    AllocConsole();
    freopen("CONOUT$", "w", stdout); // Take the output
    freopen("CONIN$", "r", stdin);  // Read the input
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

}

void StopConsole() {
    if (hConsole == NULL) {
        return;
    }

    // Release console
    FreeConsole();
    hConsole = NULL;
}
//---

void setCollectionDuration() {
    printf("Enter data collection duration in minutes: ");
    int duration;
    scanf("%d", &duration);
    if (duration > 0) {
        collectionDurationMinutes = duration;
        DWORD currentTime = GetTickCount();
        endTime = currentTime + (collectionDurationMinutes * MINUTE_MS);
        printf("Data collection duration set to %d minutes. Program will stop after this time.\n", collectionDurationMinutes);
        StopConsole();
    } else {
        printf("Invalid duration. Keeping the previous value: %d minutes.\n", collectionDurationMinutes);
    }
}

//The following three functions are in a beta-state, they were included as an outlook to future addtions, in specific a Windows Message Box as Input-Handler 

// This version of setCollectionDuration() is needed for the future usage of message boxes 
// void setCollectionDuration() {
//     char input[32];
//
//     if (GetInputDialog("Enter data collection duration in minutes:", input, sizeof(input))) {
//         int duration = atoi(input);
//         if (duration > 0) {
//             collectionDurationMinutes = duration;
//             DWORD currentTime = GetTickCount();
//             endTime = currentTime + (collectionDurationMinutes * MINUTE_MS);
//             MessageBox(NULL, "Data collection duration set successfully.", "Info", MB_OK | MB_ICONINFORMATION);
//         } else {
//             MessageBox(NULL, "Invalid duration. Keeping the previous value.", "Error", MB_OK | MB_ICONERROR);
//         }
//     }
// }

// Helper function to display input dialog
// bool GetInputDialog(char* prompt, char* output, int outputSize) {
//     char input[32] = {0};

//     // Dialog aufrufen
//     if (DialogBoxParam(
//             GetModuleHandle(NULL),
//             MAKEINTRESOURCE(IDD_INPUT_DIALOG),
//             NULL,
//             InputDialogProc, // Statt Lambda die Callback-Funktion
//             (LPARAM)input) == IDOK) {

//         strncpy(output, input, outputSize - 1);
//         return true;
//     }
//     return false;
// }

// Callback function for Window-Call with Input-Field in normal Windows-API scheme
// INT_PTR CALLBACK InputDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
//     switch (uMsg) {
//     case WM_INITDIALOG:
//         // Initialisierungen, falls erforderlich
//         return TRUE;

//     case WM_COMMAND:
//         if (LOWORD(wParam) == IDOK) {
//             GetDlgItemText(hwndDlg, IDC_EDIT_INPUT, (LPSTR)lParam, 32);
//             EndDialog(hwndDlg, IDOK);
//             return TRUE;
//         } else if (LOWORD(wParam) == IDCANCEL) {
//             EndDialog(hwndDlg, IDCANCEL);
//             return TRUE;
//         }
//         break;
//     }

//     return FALSE;
// }

// Logging thread to handle data storage for every minute and calculations every 15 minutes
void LogThread(void *param)
{
    DWORD lastLogTime = GetTickCount();
    DWORD lastCalculationTime = lastLogTime;

    while (running)
    {
        DWORD currentTime = GetTickCount();

        // Stop the thread if the specified duration has passed
        if (endTime > 0 && currentTime >= endTime) { // Only stops if endTime is set
            StartConsole();
            printf("Data collection duration (%d minutes) has elapsed. Stopping program.\n", collectionDurationMinutes);
            StopConsole();
            running = false; // Signal to stop the program
            break;
        }

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
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {  // pKeyInfo->vkCode == VK_CONTROL
                printf("Ctrl pressed\n");
                ctrlPressed = true;
                //writeResultsToLog(1,1,1,1,1,1,1,1,42);
                if (ctrlPressed && (GetAsyncKeyState(VK_SHIFT) & 0x8000) && pKeyInfo->vkCode == 'K') {
                StartConsole();
                printf("Ctrl + Shift + K pressed\n"); // Debug
                ReleaseMutex(mutex); // Unlock before opening the dialog
                setCollectionDuration(); // Trigger user prompt
                WaitForSingleObject(mutex, INFINITE); // Re-lock after dialog
                //writeResultsToLog(1,1,1,1,1,1,1,1,43);
                }
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
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000) { // pKeyInfo->vkCode == VK_CONTROL
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

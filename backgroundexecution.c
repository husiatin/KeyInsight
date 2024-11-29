#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Starts the program in background mode by opening it in a new window.
 * This starts a new background process and exits the current instance.
 */
void RunInBackground() {
    // Gets the current process handle
    char szPath[MAX_PATH];
    if (GetModuleFileName(NULL, szPath, MAX_PATH) == 0) {
        printf("Error: Can not access the path of the module.\n");
        exit(1);
    }

    // Creates a new instance of the process with `CREATE_NO_WINDOW`, so that the process runs in the background
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si); // Setting the structure size

    if (!CreateProcess(
            szPath,         // Current process
            NULL,           // Uses the string pointed to by szPath as the command line
            NULL,           // No inheritance of the process object handle to child process
            NULL,           // No inheritance of the thread object handle to child process
            FALSE,          // Handles are not inherited
            CREATE_NO_WINDOW, // No console window is opened
            NULL,           // Use environment without changes
            NULL,           // Use working directory without changes
            &si,            // Initialisation information
            &pi             // Identification information about the new process
        )) {
        printf("Error: Background process could not be started (%d).\n", GetLastError());
        exit(1);
    }

    // Original process can now be stopped
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    exit(0);
}

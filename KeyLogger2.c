#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <process.h> // Für _beginthread()

// Globale Variablen
FILE *logfile;
int keyCount = 0;           // Zähler für Tastenanschläge
int enterCount = 0;         // Zähler für Enter-Taste
int backspaceCount = 0;     // Zähler für Backspace-Taste
bool running = true;        // Kontrolliert den Logging-Thread

// Hier werden Tastenanschläge jede 5 Sec in die Log-Datei geschrieben (5 Sec nur als Beispiel)
void LogThread(void *param) {
    DWORD lastLogTime = GetTickCount();

    while (running) {
        DWORD currentTime = GetTickCount();

        // Prüfen, ob 5 Sekunden (5.000 ms) vergangen sind
        if (currentTime - lastLogTime >= 5000) {
            // Anzahl der Tastenanschläge pro 5 Sec ins Log schreiben
            fprintf(logfile, "Tastenanschläge: %d\n", keyCount);
            fprintf(logfile, "Enter-Tasten: %d\n", enterCount);
            fprintf(logfile, "Backspace-Tasten: %d\n\n", backspaceCount);
            fflush(logfile); // Schreibt Puffer-Inhalt sofort in die Datei ohne zu warten, bis es voll ist und leert den Puffer 

            // Zähler zurücksetzen und letzte Log-Zeit aktualisieren
            keyCount = 0;
            enterCount = 0;
            backspaceCount = 0;
            lastLogTime = currentTime;
        }

        // Kurze Pause, um CPU-Last zu reduzieren
        Sleep(100);
    }

    _endthread();
}

// Hier wird geschaut, ob eine Taste gedrückt wurde 
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) { // Zeigt an, dass eine Eingabeaktion stattgefunden hat
        // Wenn eine Taste gedrückt wird
        if (wParam == WM_KEYDOWN) {
            // Zugriff auf die Tasteninformationen
            KBDLLHOOKSTRUCT *pKeyInfo = (KBDLLHOOKSTRUCT *)lParam;

            // Allgemeine Zählung für Tastenanschläge
            keyCount++;

            // Überprüfen, ob Enter oder Backspace gedrückt wurde
            if (pKeyInfo->vkCode == VK_RETURN) {
                enterCount++; // Enter-Taste
            } else if (pKeyInfo->vkCode == VK_BACK) {
                backspaceCount++; // Backspace-Taste
            }
        }
    }
    //  Sorgt dafür, dass das Ereignis an andere Hooks oder Standardverarbeitungsroutinen weitergegeben wird.
    return CallNextHookEx(NULL, nCode, wParam, lParam); 
}

int main() {
    // Logdatei öffnen
    logfile = fopen("keylog.txt", "a");
    if (!logfile) {
        printf("Konnte Logdatei nicht öffnen.\n");
        return 1;
    }

    // Start des Logging-Threads
    _beginthread(LogThread, 0, NULL);

    // Hook setzen
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!keyboardHook) {
        printf("Konnte Keyboard-Hook nicht setzen.\n");
        fclose(logfile);
        running = false; // Beende den Thread
        return 1;
    }

    // Nachrichten-Schleife, die Systemereignisse überwacht
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Hook entfernen und Logdatei schließen
    UnhookWindowsHookEx(keyboardHook);
    running = false; // Beende den Thread
    fclose(logfile);

    return 0;
}

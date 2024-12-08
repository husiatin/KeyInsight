// Contributers: Jannis Neuhaus, Andreas Lerch


#define UNICODE
#define _UNICODE

#include <windows.h>
#include <shlobj.h>    // For CSIDL_STARTUP
#include <stdio.h>
#include <shobjidl.h>  // For IShellLink and IPersistFile
#include <objbase.h>   // For CoInitialize and CoUninitialize

void createShortcut(LPCWSTR shortcutPath, LPCWSTR targetPath, LPCWSTR description, LPCWSTR workingDirectoryPath) {
    HRESULT hRes;
    IShellLinkW* pShellLink = NULL;

    // Initialize COM
    hRes = CoInitialize(NULL);
    if (FAILED(hRes)) {
        wprintf(L"COM initialization failed: 0x%X\n", hRes);
        return;
    }

    // Create IShellLink object
    hRes = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (void**)&pShellLink);
    if (SUCCEEDED(hRes)) {
        // Set target path, description, and working directory
        pShellLink->lpVtbl->SetPath(pShellLink, targetPath);
        pShellLink->lpVtbl->SetDescription(pShellLink, description);
        pShellLink->lpVtbl->SetWorkingDirectory(pShellLink, workingDirectoryPath);

        // Query IPersistFile interface
        IPersistFile* pPersistFile;
        hRes = pShellLink->lpVtbl->QueryInterface(pShellLink, &IID_IPersistFile, (void**)&pPersistFile);
        if (SUCCEEDED(hRes)) {
            // Save the shortcut
            hRes = pPersistFile->lpVtbl->Save(pPersistFile, shortcutPath, TRUE);
            if (SUCCEEDED(hRes)) {
                wprintf(L"Shortcut successfully created: %ls\n", shortcutPath);
            } else {
                wprintf(L"Failed to save the shortcut: 0x%X\n", hRes);
            }
            pPersistFile->lpVtbl->Release(pPersistFile);
        } else {
            wprintf(L"QueryInterface for IPersistFile failed: 0x%X\n", hRes);
        }
        pShellLink->lpVtbl->Release(pShellLink);
    } else {
        wprintf(L"Failed to create IShellLink: 0x%X\n", hRes);
    }

    // Uninitialize COM
    CoUninitialize();
}


void createShortcutRegistry(LPCWSTR shortcutPath, LPCWSTR targetPath, LPCWSTR description){

}

int getDetails(){
    // Pfad des aktuellen .exe (setup.exe)
    WCHAR currentPath[MAX_PATH];
    if (GetModuleFileNameW(NULL, currentPath, MAX_PATH) == 0) {
        wprintf(L"Fehler beim Abrufen des aktuellen Pfads: %lu\n", GetLastError());
        return 1;
    }

    // Arbeitsverzeichnis erhalten, indem 'setup.exe' vom 'currentPath' entfernt wird
    WCHAR workingDirectoryPath[MAX_PATH];
    wcscpy_s(workingDirectoryPath, MAX_PATH, currentPath);

    WCHAR* lastBackslash = wcsrchr(workingDirectoryPath, L'\\');
    if (lastBackslash != NULL) {
        // String am letzten Backslash terminieren, um den Dateinamen zu entfernen
        *lastBackslash = L'\0';
    } else {
        wprintf(L"Fehler beim Parsen des Pfads: %ls\n", workingDirectoryPath);
        return 1;
    }

    // Pfad zu KeyInsight.exe (im selben Ordner wie setup.exe)
    WCHAR KeyInsightPath[MAX_PATH];
    swprintf_s(KeyInsightPath, MAX_PATH, L"%s\\KeyInsight.exe", workingDirectoryPath);

    // Pfad zum Autostart-Ordner
    WCHAR startupPath[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath) != S_OK) {
        wprintf(L"Fehler beim Abrufen des Autostart-Ordners.\n");
        return 1;
    }

    // Pfad zum Shortcut im Autostart
    WCHAR shortcutPath[MAX_PATH];
    swprintf_s(shortcutPath, MAX_PATH, L"%s\\KeyInsight.lnk", startupPath);

    // Shortcut erstellen mit dem Arbeitsverzeichnis
    createShortcut(shortcutPath, KeyInsightPath, L"Autostart for KeyInsight.exe", workingDirectoryPath);

    return 0;
}

void clearInputBuffer() {
    wint_t c;
    while ((c = getwchar()) != L'\n' && c != WEOF);
}

int main() {
    wchar_t choice;

    // User prompt
    do {
        wprintf(L"Do you want to create a shortcut in the startup folder? [Y/n]: ");
        choice = getwchar();

        if (choice == L'\n' || choice == WEOF) {
            choice = L'y'; // Default to 'y'
        } else {
            clearInputBuffer(); // Clear input buffer
        }

        if (choice == L'y' || choice == L'Y') {
            wprintf(L"You selected 'Yes'. Shortcut will be created.\n");
            getDetails();
            wprintf(L"Press any key to close this window.\n");
            choice = getwchar();
            break;
        } else if (choice == L'n' || choice == L'N') {
            wprintf(L"You selected 'No'. No Shortcut created.\n");
            wprintf(L"Press any key to close this window.\n");
            choice = getwchar();
            return 0; // Exit program
        } else {
            wprintf(L"Invalid input. Please enter 'y' or 'n'.\n");
        }
    } while (1);

    return 0;
}

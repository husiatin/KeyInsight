Contributers: Andreas Lerch, Dustin Lojewski, Jannis Neuhaus
# KeyInsight
Bei dem erstmaligen Ausführen der setup.exe und KeyInsight.exe kann es dazu kommen, dass Windows aufgrund des unbekannten Herausgebers verhindert die Anwendungen auszuführen.
Beim Verhindern durch Windows wird eine Meldung angezeigt, diese kann durch drücken auf "Weitere Informationen" und dann "Trotzdem Ausführen" gelöst werden,
wodurch dann auch die Anwendungen ordnungsgemäß ausgeführt werden.

Um Kompilierung zu ermöglichen, diese Befehle der tasks.json hinzufügen.

tasks.json

"args": [
    ...
    "-lole32",
    "-lshell32",
    "-luuid"
    "-mwindows"
   ]

Um die setup.exe ausführen zu können werden Adminrechte benötigt.
Durch das Ausführen der setup.exe wird KeyInsight für den automatischen Start bei Hochfahren des Computer konfiguriert.
Das KeyInsight beim nächsten Start des Computers ordnungsgemäß gestartet wird muss KeyInsight einmal ausgeführt werden.

Beim Doppelklick auf die KeyInsight.exe wird KeyInsight gestartet und läuft im Hintergrund.

Die gesammlten und ausgewerteten Daten werden alle 15 Minuten in einer .log Datei und einer .yaml Datei gespeichert.
Diese Dateien werden im selben Ordner in dem auch die KeyInsight.exe liegt gespeichert.

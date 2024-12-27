Contributers: Andreas Lerch, Dustin Lojewski, Jannis Neuhaus
# KeyInsight
tasks.json Argumente für das Kompiliern der KeyInsight.c mit VS Code:

"args": [
    ...
    "-lole32",
    "-lshell32",
    "-luuid"
    "-mwindows"
   ]

Bei dem erstmaligen Ausführen der setup.exe und KeyInsight.exe kann es dazu kommen, dass Windows aufgrund des unbekannten Herausgebers verhindert die Anwendungen auszuführen.
Beim Verhindern durch Windows wird eine Meldung angezeigt, diese kann durch drücken auf "Weitere Informationen" und dann "Trotzdem Ausführen" gelöst werden,
wodurch dann auch die Anwendungen ordnungsgemäß ausgeführt werden.

Um Kompilierung zu ermöglichen, diese Befehle der tasks.json hinzufügen.

Um die setup.exe ausführen zu können werden Adminrechte benötigt.
Durch das Ausführen der setup.exe wird KeyInsight für den automatischen Start bei Hochfahren des Computer konfiguriert.
Zusätzlich muss KeyInsight einmalig ausgeführt werden, um den ordnungsgemäßen Autostart bei Hochfahren des Computers zu ermöglichen.

Beim Doppelklick auf die KeyInsight.exe wird KeyInsight gestartet und läuft im Hintergrund.

Die gesammlten und ausgewerteten Daten werden alle 15 Minuten in einer .log Datei und einer .yaml Datei gespeichert.
Diese Dateien werden im selben Ordner in dem auch die KeyInsight.exe liegt gespeichert.

Der Ausführungszeitraum der KeyInsight Anwendung kann durch das drücken der Tastenkombination "strg + shift + k" eingestellt werden.
Dabei wird eine Kommandozeile geöffnet, in die eine Laufzeit in Minuten eingegeben werden kann.
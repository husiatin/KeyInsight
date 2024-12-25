# KeyInsight
KeyInsight wir wollen die Weltherrschafft.

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

Beim Doppelklick auf die KeyInsight.exe wird KeyInsight gestartet und läuft im Hintergrund.

Die gesammlten und ausgewerteten Daten werden alle 15 Minuten in einer .log Datei und einer .yaml Datei gespeichert.
Diese Dateien werden im selben Ordner in dem auch die KeyInsight.exe liegt gespeichert.

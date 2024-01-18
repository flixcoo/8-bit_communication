# Versuch 7 - Hardwarepraktikum II
Felix Kirchner, Quentin Kleinert
## Aufgabe 4.1
Das Programm kann einzelnde Zeichen zwischen zwei Computern hin und her schicken. Es wird auf beiden Rechnern gestartet und durch das Menü bei Eingabe einer 0 oder 1 ausgewählt, welcher Computer Sender und welcher Computer Empfänger ist. Falls eine falsche Eingabe erfolgen sollte, wird diese abgefangen und der Nutzer wird erneut nach seiner Auswahl gefragt.\
Nach der Zuteilung von Sender und Empfäger gibt man dem Sender-PC ein Zeichen. Dieser wandelt das Zeichen von einem Charactar in des ASCII-Wert um, welcher dann in Binär übersetzt wird. Die Binärumwandlung wird in drei 3-Bit-Blöcke an den anderen PC gesendet. Dieser fügt die Blöcke wieder zusammen und gibt sie als Binär und als ASCII-Zeichen aus.\
Danach gelangt man wieder ins Menü und es kann von vorne begonnen werden.
## Aufgabe 4.2
Anstatt der normalen Menüführung werden nun Linux-Pipes verwendet. Beim Empfänger-PC muss nach dem Starten des Programms lediglich einmal die Enter-Taste gedrückt werden, um den Empfangsmodus zu aktivieren. Beim Sender-PC kann über den Befehl `echo "Zusendender Text" | ./main.elf` das Programm gestartet werden.\
Im Gegensatz zu 4.1 wird nun vor der eigentlichen Übertragung nun noch die Länge der Zeichenkette mit übertragen. Diese wird direkt nach dem ESC-Zeichen gesendet um dem Empfänger mitzuteilen, wie viele Zeichen er mitlesen muss. Das weitere Vorgehen ist dann wie in 4.2. Die einzelnen Zeichen werden an eine Stringvariable angefügt welche dann am Ende ausgegeben wird.
## Aufgabe 4.3
tbc
## Aufgabe 4.4
tbc
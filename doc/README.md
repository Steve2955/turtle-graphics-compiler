@mainpage Projektbeschreibung
@author Yannis Becker
@author Luis Schäfer
@author Christopher Kloß

@section sec_overview Überblick

Dieses Projekt wurde als Klausurersatzleistung im Rahmen des Moduls "Compilerbau" erstellt. Es handelt sich um einen Compiler für eine einfach Programmiersprache zum Zeichnen von Strichgrafiken. Auf der Grundlage eines vorgegebenen Evaluators wurden der Lexer und der Parser entwickelt.

Der Lexer befindet sich in der Datei `lex.c`. Dieser wurde logisch komplett vom Parser getrennt. Dazu wurde `token_t` als eigener Datenstruktur für Tokens definiert. Diese Datenstruktur ist als zweifach verkettete Liste implementiert.

Der Parser befindet sich in der Datei `parse.c`. Dieser ruft selbstständig den Lexer auf. Die Token-Liste des Lexer wird in einen Syntaxbaum auf Basis der vorgegebenen Datenstruktur `treenode_t` umgewandelt, die dann vom vorgefertigten Evaluator verarbeitet werden kann.

@subsection sec_usage_build Benutzung der Build-Skripts

Zum Bauen des Projektes wurden Build-Skripte benutzt (`build.cmd`, `build.sh`). Im Folgenden ist der unter Linux verwendete Befehl gelistet.

```bash
gcc `sdl2-config --cflags --libs` -lm -o build/turtle src/sdl/sdlinterf.c src/main.c src/eval.c src/parse.c src/lex.c src/nametab.c -lSDL2main -lSDL2 -Wl,--no-undefined -lm -luuid -static-libgcc
```

@subsection sec_usage_prog Benutzung des Compilers

```
turtle Programm-Datei [Zahlen ...]
```

@section sec_course Verlauf des Projektes

@subsection sec_problems Schwierigkeiten

- Umfang des Projektes ist dem Stundenumfang des Moduls nicht gerecht
- Aufteilung in einzelne Abschnitte (abseits von Parser und Lexer) schwierig; daher oftmals im Pair-Programming umgesetzt

@subsection sec_improve Verbesserungsvorschläge

- Bessere Dokumentation der bereitgestellten Files (es war z.B. nicht immer ganz klar in welcher Struktur der Evaluator Daten erwartet)

@subsection sec_joy Freude

- die Low-Level Arbeit mit C hat mal wieder Spaß gemacht, da es in unserem Umfeld sonst fast nie zum Einsatz kommt
- das Innenleben eines Compilers bis in die Tiefe kennenzulernen war definitv ein Highlight
- für einige Gruppenmitglieder war die Entwicklung eines solchen Projektes im Team neu und Erfahrungen in der Nutzung von Tools wie `git` (und Github) wurden daher gerne geteilt
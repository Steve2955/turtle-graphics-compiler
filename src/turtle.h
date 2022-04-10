
// Turtle-Graphics-Compiler:
// Gemeinsamer Header-File für globale Funktionen und Variablen
//
// Klaus Kusche 2021, 2022

#ifndef _TURTLE_H
#define _TURTLE_H

#include <stdio.h>

#include "types.h"

///////////////////////////////////////////////////////////////////////////////
// turtle-main.c
///////////////////////////////////////////////////////////////////////////////

// Der Sourcefile
extern FILE *src_file;
// Globaler Programmname für Fehlermeldungen
extern const char *prog_name;

// Prüfe ob malloc/calloc/realloc erfolgreich war:
// Fehlermeldung und Programmende wenn p gleich NULL ist
// what ... was wurde gerade angelegt?
// pos ... für welche Stelle im Source?
extern void mem_check(const void *p, const char *what, const srcpos_t *pos);

// Ausgabe eines Fehlers im Turtle-Programm an Stelle pos & Programmende
// (mit variabel vielen Parametern wie printf)
void code_error(const srcpos_t *pos, const char *format, ...);

///////////////////////////////////////////////////////////////////////////////
// turtle-nametab.c
///////////////////////////////////////////////////////////////////////////////

// Größe der Namens-Tabelle
#define MAX_NAMES 1000

// die zentrale Namenstabelle
extern int nameCount;
extern nameentry_t name_tab[MAX_NAMES];

///////////////////////////////////////////////////////////////////////////////
// turtle-parse.c
///////////////////////////////////////////////////////////////////////////////

// Sourcefile komplett einlesen, lexen und parsen
// Die Syntaxbäume von Funktionen werden in der Namenstabelle eingetragen
// Der Returnwert ist der Syntaxbaum des Hauptprogrammes
extern treenode_t *parse(void);

///////////////////////////////////////////////////////////////////////////////
// turtle-eval.c
///////////////////////////////////////////////////////////////////////////////

// Speicherplätze für die vordefinierten globalen Variablen
// (benötigt in turtle-nametab.c)
extern double g_dir;       // Richtung (in Grad, wie in Mathe:
                           // 0 ist nach rechts, gegen den Uhrzeigersinn)
extern double g_dist;      // Abstand vom Ursprung
extern double g_x, g_y;    // Aktuelle x- und y-Position
extern double g_args[10];  // argv[1]...argv[9] als double
                           // g_args[0] bleibt unbenutzt
extern double g_pi;        // Die Konstante Pi
extern double g_max_x, g_max_y; // Größe des Fensters in Benutzer-Koordinaten
extern double g_delay;     // Verzögerung in ms nach jedem Zeichnen
extern double g_red, g_green, g_blue; // Farbe des Striches

// Die Hauptfunktion des Evaluators:
// Initialisiere die Verarbeitung und die Grafik
// und arbeite den Syntaxbaum main_tree für das Haupprogramm ab
// arg_cnt/arg_val ist der Teil von argc/argv, der in @1, @2, ... gehört
// (ohne Programmname und Sourcefile-Name)
extern void evaluate(const treenode_t *main_tree,
                     int arg_cnt, const char *arg_val[]);



// ============================================================================

extern token_t *readTokensFromFile(FILE *file);

#endif

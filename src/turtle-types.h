
// Turtle-Graphics-Compiler:
// Gemeinsamer Header-File für die Typen für Nametable und Syntaxbaum
//
// Klaus Kusche 2021, 2022

#ifndef _TURTLE_TYPES_H
#define _TURTLE_TYPES_H

#include <stdbool.h>

// Max. Anzahl von Funktions-Argumenten
#define MAX_ARGS 8

// Unsere "Universal-Enumeration": Konstanten für die Art ...
// * ... eines Tokens (turtle-parse: Globale Variable tok)
// * ... eines Eintrags in der Namens-Tabelle (nameentry_t Member type)
// * ... eines Syntaxbaum-Knotens (treenode_t Member type)
typedef enum {
///////////////////////////////////////////////////////////////////////////////
// innerhalb des Programms definierte Namen
///////////////////////////////////////////////////////////////////////////////
  // In Token: Alle Namen werden vom Lexer als name_any-Token geliefert
  // In der Namenstabelle: Solange noch nicht feststeht,
  //   ob ein neuer Name als name_var, name_path oder name_calc verwendet wird,
  //   bekommt er Namenstyp name_any
  // Im Syntaxbaum:
  //   - Lesende Zugriffe auf Variablen in Rechnungen
  //   werden im Syntaxbaum durch name_any-Knoten repräsentiert
  //   (egal ob der Name ein name_var, name_glob oder name_pvar ist)
  //   - Funktionsaufrufe bekommen im Syntaxbaum einen oper_lpar-Knoten,
  //   sowohl für benutzerdefinierte als auch für vordefinierte Funktionen
  name_any = 0,  // beliebiger Name
  name_var,      // lokale Variable oder Parameter (werden nicht unterschieden!)
  name_path,     // Name einer Path-Funktion
  name_calc,     // Name einer Calc-Funktion
  name_glob,     // benutzerdefinierte globale Variable
///////////////////////////////////////////////////////////////////////////////
// vordefinierte globale Variablen
///////////////////////////////////////////////////////////////////////////////
  name_pvar_ro,      // nur lesbar      
  name_pvar_rw,      // lesbar und schreibbar
///////////////////////////////////////////////////////////////////////////////
// vordefinierte Funktionen
///////////////////////////////////////////////////////////////////////////////
  name_math_sin, name_math_cos, name_math_tan, name_math_sqrt,
  name_math_rand,
///////////////////////////////////////////////////////////////////////////////
// Keywords
///////////////////////////////////////////////////////////////////////////////
  keyw_dummy,  // Dummy, Beginn der Keywords in der enum
  keyw_walk,
  keyw_jump,
  keyw_back,
  keyw_home,
  keyw_turn,
  keyw_left,
  keyw_right,
  keyw_direction,
  keyw_clear,
  keyw_stop,
  keyw_finish,
  keyw_path,
  keyw_color,
  keyw_store,
  keyw_in,
  keyw_add,
  keyw_to,
  keyw_sub,
  keyw_from,
  keyw_mul,
  keyw_by,
  keyw_div,
  keyw_mark,
  keyw_if,
  keyw_then,
  keyw_else,
  keyw_endif,
  keyw_do,
  keyw_times,
  keyw_done,
  keyw_counter,
  keyw_downto,
  keyw_step,
  keyw_while,
  keyw_repeat,
  keyw_until,
  keyw_endpath,
  keyw_calculation,
  keyw_returns,
  keyw_endcalc,
  keyw_begin,
  keyw_end,
  keyw_and,
  keyw_or,
  keyw_not,
///////////////////////////////////////////////////////////////////////////////
// Tokens, die nicht Namen/Keywords sind
///////////////////////////////////////////////////////////////////////////////
  tok_bofeof,  // Beginning & End of file (nur als Tokens)
  oper_const,  // Zahl
  oper_lpar,   // (, im Syntaxbaum: Funktionsaufruf
  oper_rpar,   // )
  oper_sep,    // ,
  oper_abs,    // |
  oper_pow,    // ^
  oper_mul,    // *
  oper_div,    // /
  oper_add,    // +
  oper_sub,    // Token: Unäres oder binäres -, Syntaxbaum: Binäres -
  oper_neg,    // Syntaxbaum: Unäres -
  oper_equ,    // =
  oper_nequ,   // <>
  oper_less,   // <
  oper_lequ,   // <=
  oper_grtr,   // >
  oper_gequ,   // >=
} type_t;


// Vorab-typedef's für alle folgenden struct's und union's
typedef struct _srcpos srcpos_t;
typedef struct _funcdef funcdef_t;
typedef union _namedata namedata_t;
typedef struct _nameentry nameentry_t;
typedef union _nodedata nodedata_t;
typedef struct _treenode treenode_t;

// Position eines Tokens oder Syntax-Knotens im Source-File (Zeile / Spalte)
struct _srcpos {
  unsigned short int line, col;   // Zeilennummer und Spaltennummer
};

// Daten einer Funktions- oder Pfaddefinition: Parameternamen und Code
// (hängt am Funktions- oder Pfadnamen in der Namenstabelle)
struct _funcdef {
  treenode_t *body; // Pointer auf den Syntaxbaum des Funktionsrumpfes
                    // für name_calc ohne Rumpf: NULL
  treenode_t *ret;  // für name_calc: Pointer auf den Syntaxbaum des Returnwerts
                    // für name_path: NULL
  nameentry_t *params[MAX_ARGS]; // Pointer auf die Namenseinträge der Parameter
};

// Unterschiedliche Nutzdaten eines Eintrags in der Namens-Tabelle,
// je nach Art des Namens
union _namedata {
  double val;       // für name_glob: Aktueller Wert der globalen Variable
  funcdef_t *func;  // für name_path und name_calc:
                    //   Pointer auf die Funktionsdaten
  double *p_val;    // für name_pvar_...: Pointer auf den Variablenwert
                    // (d.h. auf die globale Variable im Evaluator)
  double (*math)(double); // für name_math_... außer name_math_rand:
                          // Function Pointer auf die math.h-Funktion
  // leer für name_var, name_any, name_math_rand und keyw_...
};

// Typ eines Eintrags in der Namens-Tabelle
struct _nameentry {
  type_t type;      // Art des Namens
  const char *name; // Namens-String (bei name_glob und name_pvar_... *mit* @):
                    // Pointer auf den mit strdup dynamisch gespeicherten Namen
  namedata_t d;     // Daten zum Namen
};

// Unterschiedliche Nutzdaten eines Syntaxbaum-Knotens,
// je nach Art des Knotens
union _nodedata {
  nameentry_t *p_name; // für jeden Knoten mit Variablen- oder Funktionsnamen:
                       // Pointer auf den Namens-Eintrag
  double val;          // für Zahl-Konstanten: Wert der Zahl
  type_t walk;         // Art von walk und jump: keyw_back, keyw_home, keyw_mark
                       // oder keyw_walk für normales walk/jump,
  // leer für alle anderen Knotentypen
};

// Im Syntaxbaum werden folgende Knotentypen verwendet,
// mit folgenden Unter-Syntaxbäumen im son-Array (keine wenn nichts angegeben)
// und folgender Information im Art-abhängigen Member d
//
// name_any     Variable (lesend in einer Rechnung): d = Namenseintrag (p_name)
// oper_const   Zahl: Konstante, d = Wert (val)
// oper_neg     unäres -: son 0 = Operand
// oper_pow, oper_mul, oper_div, oper_add, oper_sub
//              ^ * / + -: son 0 und son 1 = Operanden
// oper_abs,    Absolutbetrag: son 0 = Operand
// oper_lpar    '(' = Funktionsaufruf:
//              d = Namenseintrag der Funktion (p_name)
//              son 0, 1, ...: Argumente (nicht vorhandene Argumente sind NULL)
// oper_equ, oper_nequ, oper_less, oper_lequ, oper_grtr, oper_gequ
//              = <> < <= > >=: son 0 und son 1 = Operanden   
// oper_not     Logisches not: son 0 = Operand
// oper_and, oper_or  Logisches and und or: son 0 und son 1 = Operanden  
// keyw_walk, keyw_jump
//              walk oder jump in allen Kombinationen
//              d = Art des walk: keyw_walk, keyw_back, keyw_home, keyw_mark
//              bei walk/jump und walk/jump back: son 0 = expr
// keyw_right   turn und turn right: son 0 = expr
// keyw_left    turn left: son 0 = expr
// keyw_direction  direction: son 0 = expr
// keyw_clear, keyw_stop, keyw_finish, keyw_mark
//              clear, stop, finish, mark: Keine Operanden  
// keyw_path    path-Aufruf: d = Name der Pfadfunktion, son 0, 1, ...: Argumente
//              nicht vorhandene Argumente sind NULL
// keyw_color   color: son 0 = rot, son 1 = grün, son 2 = blau
// keyw_store, keyw_add, keyw_sub, keyw_mul, keyw_div
//              store, add, sub, mul, div:
//              d = Namens-Eintrag der Variable (p_name),
//              son 0 = expr
// keyw_if      if: son 0 = cond, son 1 = then-statements,
//              son 2 = else-statements oder NULL
// keyw_do,     times-Schleife: son 0 = expr, son 1 = statements
// keyw_counter counter-Schleife:
//              d = Namens-Eintrag der Variable (p_name),
//              son 0 = Startwert
//              to: son 1 = Endwert, son 2 = NULL
//              downto: son 1 = NULL, son 2 = Endwert
//              son 3 = Schrittwert oder NULL
//              son 4 = statements
// keyw_while   while-Schleife: son 0 = cond, son 1 = statements
// keyw_repeat  repeat-Schleife: son 0 = cond, son 1 = statements
//
// Vorrang-Klammern in Rechnungen und Bedingungen
// sind *keine* eigenen Knoten im Syntaxbaum
// sondern beeinflussen beim Parsen nur die Struktur des Syntaxbaumes

// Typ eines Syntaxbaum-Knotens
struct _treenode {
  type_t type;      // Art des Knotens
  srcpos_t pos;     // Position im Sourcefile (für Fehlermeldungen)
  treenode_t *next; // Bei allen Statements: Nächstes Statement
                    // (oder NULL beim letzten Statement einer Statement-Liste)
                    // Bei allen anderen Knoten: NULL
  treenode_t *son[MAX_ARGS]; // Sohn-Syntaxbäume
  nodedata_t d;     // Daten zum Knoten
};

#endif

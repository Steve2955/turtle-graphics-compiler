#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "turtle.h"
#include "types.h"

treenode_t *expression();
treenode_t *statements();
treenode_t *condition();

/// aktueller Token
token_t *token;

/// aktuellen Token auf den Nächsten setzen
token_t *next() {
	if(token->next != NULL) printf("Token: %s\n", token->next->tok);
	return token = token->next;
}

/// aktuellen Token auf den Vorherigen setzen
token_t *prev() {
	return token = token->prev;
}

/// Überprüfung, ob der aktuelle Token dem erwartetem Typen entspricht -> Fehlermeldung + Programmabbruch wenn nicht
void expectTokenType(type_t expected, char *error){
	if(token == NULL || token->type != expected){
		fprintf(stderr, "Unerwarteter Token-Typ: %s\n", error);
		exit(EXIT_FAILURE);
	}
}

/// Lineare Suche eines Namens in der Namenstabelle (quick and dirty Lösung -> normalerweise sind Hashtables üblich)
nameentry_t *findNameEntry(char *name){
	for(int i = 0; i < nameCount; i++){
		if(strcmp(name, name_tab[i].name) == 0){
			return &name_tab[i];
		}
	}
	return NULL;
}

/// Namenseintrag des korrekten Types des aktuellen Token in der Namenstabelle suchen (Typ wird gesetzt, falls dies noch nicht erfolgt ist)
nameentry_t *findNameEntryOfType(type_t type){
	nameentry_t *n = findNameEntry(token->tok);
	if(n->type == name_any){
		n->type = type;
	}else if(n->type != type){
		fprintf(stderr, "Namenseintrag bereits al anderer Typ vorhanden\n"); // ToDo
		exit(EXIT_FAILURE);
	}
	return n;
}

nameentry_t *findVarNameEntry(){
	nameentry_t *n = findNameEntry(token->tok);
	expectTokenType(name_any, "Variable erwartet");
	if(n->type == name_any){
		n->type = name_var;
	}
	if(n->type != name_var && n->type != name_glob && n->type != name_pvar_rw && n->type != name_pvar_ro){
		fprintf(stderr, "Namenseintrag bereits vorhanden\n"); // ToDo
		exit(EXIT_FAILURE);
	}
	return n;
}

/// Parsen von Argument-Listen
treenode_t *args(treenode_t *f){
	expectTokenType(oper_lpar, "'(' erwartet");
	next();
	int argc = 0;
	while(token->type != oper_rpar){
		if(argc >= MAX_ARGS){
			fprintf(stderr, "Zu viele Argumente\n");
			exit(EXIT_FAILURE);
		}
		treenode_t *a = expression();
		f->son[argc] = a;
		argc++;
		if(token->type == oper_sep){
			next();
			continue;
		}else{
			expectTokenType(oper_rpar, "')' erwartet");
		}
	}
	next();
	return f;
}

/// Parsen von Operanden
treenode_t *operand(){
	treenode_t *a = malloc(sizeof(treenode_t));
	switch(token->type){
		case oper_const:
			a->d.val = atof(token->tok);
			a->type = token->type;
			next();
			return a;
		case name_any:
			a->type = token->type;
			nameentry_t *n = findNameEntry(token->tok);
			n->type = name_var;
			a->d.p_name = n;
			next();
			return a;
		case name_glob:
		case name_var:
		case name_pvar_ro:
		case name_pvar_rw:
			a->d.p_name = findNameEntry(token->tok);
			next();
			return a;
		case name_math_sin:
		case name_math_cos:
		case name_math_tan:
		case name_math_sqrt:
		case name_math_rand:
			a->type = oper_lpar;
			a->d.p_name = findNameEntry(token->tok);
			next();
			a = args(a);
			return a;
		case oper_abs:
			a->type = token->type;
			next();
			a->son[0] = expression();
			expectTokenType(oper_abs, "\"|\" erwartet");
			next();
			return a;
		case oper_sub:
			a->type = oper_neg;
			next();
			a->son[0] = expression();
			return a;
	}
	printf("unknown type: %d\n", token->type); //ToDo: Error
	return NULL;
}

/// Parsen von Faktoren
treenode_t *faktor(){
	treenode_t *a = operand();
	if(token->type != oper_pow) return a;
	treenode_t *pow = malloc(sizeof(treenode_t));
	pow->type = token->type;
	next();
	pow->son[0] = a;
	pow->son[1] = faktor();
	return pow;
}

/// Parsen von Termen
treenode_t *term(){
	treenode_t *a = faktor();
	while(token->type == oper_mul || token->type == oper_div){
		treenode_t *b = malloc(sizeof(treenode_t));
		b->type = token->type;
		next();
		b->son[0] = a;
		b->son[1] = faktor();
		a = b;
	}
	return a;
}

/// Parsen von Wahrheitswerten (ToDo)
treenode_t *val(){
	treenode_t *a = malloc(sizeof(treenode_t));
	if(token->type == oper_lpar){
		next();
		a = condition();
		expectTokenType(oper_rpar, "')' erwartet");
		next();
		return a;
	}
	if(token->type == keyw_not){
		a->type = token->type;
		next();
		a->son[0] = val();
		a->pos = token->pos;
		return a;
	}
	a = expression();
	a->pos = token->pos;
	switch(token->type){
		case oper_equ:
		case oper_nequ:
		case oper_less:
		case oper_lequ:
		case oper_grtr:
		case oper_gequ:
			treenode_t *b = malloc(sizeof(treenode_t));
			b->type = token->type;
			b->pos = token->pos;
			next();
			b->son[0] = a;
			b->son[1] = expression();
			return b;
		break;
		default:
			printf("Operator expected\n");
			exit(EXIT_FAILURE);
	}
	printf("ERROR buildung val\n");
}

/// Parsen logischer UND-Ausdrücke
treenode_t *and(){
	treenode_t *a = val();
	while(token->type == keyw_and){
		treenode_t *b = malloc(sizeof(treenode_t));
		b->type = token->type;
		next();
		b->son[0] = a;
		b->son[1] = val();
		a = b;
	}
	return a;
}

/// Parsen logischer Ausdrücke
treenode_t *condition(){
	treenode_t *a = and();
	while (token->type == keyw_or) {
		treenode_t *b = malloc(sizeof(treenode_t));
		b->type = token->type;
		next();
		b->son[0] = a;
		b->son[1] = and();
		a = b;
	}
	return a;
}

/// Parsen von mathematischen Ausdrücken
treenode_t *expression(){
	treenode_t *a = term();
	while ((token->type == oper_add) || (token->type == oper_sub)) {
		treenode_t *b = malloc(sizeof(treenode_t));
		b->type = token->type;
		next();
		// here starts the recursion madness
		b->son[0] = a;
		b->son[1] = term();
		a = b;
	}
	return a;
}

/// Parsen einzelner Anweisungen
treenode_t *statement(){
	treenode_t *statement = malloc(sizeof(treenode_t));
	switch(token->type){
		case keyw_walk:
		case keyw_jump:
			statement->type = token->type;
			next();
			if(token->type == keyw_mark){
				statement->d.walk = keyw_mark;
				next();
			}else if(token->type == keyw_home){
				statement->d.walk = keyw_home;
				next();
			}else if(token->type == keyw_back){
				statement->d.walk = keyw_back;
				next();
				statement->son[0] = expression();
			}else{
				statement->d.walk = keyw_walk;
				statement->son[0] = expression();
			}
			return statement;
		case keyw_turn:
			next();
			if (token->type == keyw_left) {
				statement->type = keyw_left;
				next();
			} else {
				statement->type = keyw_right;
				// turn "right" is optional
				if (token->type == keyw_right) next();
			}
			statement->son[0] = expression();
			return statement;
		case keyw_direction:
			next();
			statement->type = keyw_direction;
			statement->son[0] = expression();
			return statement;
		case keyw_color:
			statement->type = token->type;
			next();
			statement->son[0] = expression();
			expectTokenType(oper_sep, "\",\" erwartet");
			next();
			statement->son[1] = expression();
			expectTokenType(oper_sep, "\",\" erwartet");
			next();
			statement->son[2] = expression();
			return statement;
		case keyw_clear:
		case keyw_stop:
		case keyw_finish:
			statement->type = token->type;
			next();
			return statement;
		case keyw_store:
			statement->type = token->type;
			next();
			statement->son[0] = expression();
			expectTokenType(keyw_in, "'in' erwartet");
			next();
			statement->d.p_name = findNameEntry(token->tok);
			next();
			return statement;
		case keyw_add:
		case keyw_sub:
			statement->type = token->type;
			next();
			statement->son[0] = expression();
			if(statement->type == keyw_add){
				expectTokenType(keyw_to, "'to' erwartet");
			}else{
				expectTokenType(keyw_from, "'from' erwartet");
			}
			next();
			statement->d.p_name = findVarNameEntry(token->tok);
			next();
			return statement;
		case keyw_mul:
		case keyw_div:
			statement->type = token->type;
			next();
			statement->d.p_name = findVarNameEntry(token->tok);
			next();
			expectTokenType(keyw_by, "'by' erwartet");
			next();
			statement->son[0] = expression();
			return statement;
		case keyw_mark:
			statement->type = token->type;
			next();
			return statement;
		case keyw_do:
			statement->type = token->type;
			next();
			statement->son[0] = expression();
			expectTokenType(keyw_times, "'times' erwartet");
			next();
			statement->son[1] = statements();
			expectTokenType(keyw_done, "'done' erwartet");
			next();
			return statement;
		case keyw_counter:
			statement->type = token->type;
			next();
			statement->d.p_name = findVarNameEntry(token->tok);
			next();
			expectTokenType(keyw_from, "'from' erwartet");
			next();
			statement->son[0] = expression();
			if(token->type == keyw_downto){
				next();
				statement->son[2] = expression();
				statement->son[1] = NULL;
			}else if(token->type == keyw_to){
				next();
				statement->son[1] = expression();
				statement->son[2] = NULL;
			}else{
				printf("'to' oder 'downto' erwartet\n");
				exit(EXIT_FAILURE);
			}
			if(token->type == keyw_step){
				next();
				statement->son[3] = expression();
			}
			expectTokenType(keyw_do, "'do' erwartet");
			next();
			statement->son[4] = statements();
			expectTokenType(keyw_done, "'done' erwartet");
			next();
			return statement;
		case keyw_if:
			statement->type = token->type;
			next();
			statement->son[0] = condition();
			expectTokenType(keyw_then, "'then' erwartet");
			next();
			statement->son[1] = statements();
			if(token->type == keyw_else){
				next();
				statement->son[2] = statements();
			}else{
				statement->son[2] = NULL;
			}
			expectTokenType(keyw_endif, "'endif' erwartet");
			printf("if statement done\n");
			next();
			return statement;
		case keyw_while:
			statement->type = token->type;
			next();
			statement->son[0] = condition();
			expectTokenType(keyw_do, "'do' erwartet");
			next();
			statement->son[1] = statements();
			expectTokenType(keyw_done, "'done' erwartet");
			next();
			return statement;
		case keyw_repeat:
			statement->type = token->type;
			next();
			statement->son[1] = statements();
			expectTokenType(keyw_until, "'until' erwartet");
			next();
			statement->son[0] = condition();
			printf("repeat statement done\n");
			if(token->type == keyw_stop) printf("repeat statement with stop\n");
			return statement;
	}
	printf("statement not found\n");
}

/// Parsen einer Liste von Anweisungen
treenode_t *statements(){
	treenode_t *firstNode = NULL;
	treenode_t *currentNode = firstNode;
	while(token->type != keyw_end && token->type != keyw_endcalc && token->type != keyw_endpath  && token->type != keyw_done && token->type != keyw_endif && token->type != keyw_else && token->type != keyw_until){
		treenode_t *newNode = statement();
		if(newNode == NULL){
			fprintf(stderr, "Fehler beim Parsen des aktuellen Statements");
			exit(EXIT_FAILURE);
		}
		if(firstNode == NULL){
			currentNode = firstNode = newNode;
			currentNode->next = NULL;
		} else {
			currentNode->next = newNode;
			currentNode = newNode;
			currentNode->next = NULL;
		}
	}
	return firstNode;
}

void var();

// PARAMS ::= [ VAR { "," VAR } ]
void params();

// CALCDEF ::= "calculation" NAME "("[ PARAMS ]")" [ STATEMENTS ] "returns" EXPR "endcalc"
void calcdef(){
	expectTokenType(keyw_calculation, "\"calculation\" erwartet");
	// ToDo
	expectTokenType(keyw_endcalc, "\"endcalc\" erwartet");
	next();
}

// PATHDEF ::= "path" NAME [ "("[ PARAMS ]")" ] STATEMENTS "endpath"
void pathdef(){
	expectTokenType(keyw_path, "\"path\" erwartet");
	// ToDo
	expectTokenType(keyw_endcalc, "\"endpath\" erwartet");
	next();
}

// PROGRAM ::= { PATHDEF | CALCDEF } "begin" STATEMENTS "end"
treenode_t *program(){
	// Look for "begin" -> everything before must be a path or a calculation
	while(token != NULL && token->type != keyw_begin){
		if(token->type == keyw_path){
			pathdef();
		}else if(token->type == keyw_calculation){
			calcdef();
		}else{
			fprintf(stderr, "Unerwarteter Token-Typ: Pfad- oder Calculation-Definition erwartet\n");
			exit(EXIT_FAILURE);
		}
	}
	// "begin" found -> next
	expectTokenType(keyw_begin, "\"begin\" erwartet");
	next();
	treenode_t *prog = statements();
	// "end" expected after statements
	expectTokenType(keyw_end, "\"end\" erwartet");
	next();
	return prog;
}

/// Hauptfunktion des Parser.
/// Aufruf des Lexers, der eine bereits geöffnete Eingabe-Datei in einen Tokenstream ließt. Der Tokenstream wird danach zu einem Syntaxbaum umgewandelt.
/// @param *src_file Bereits geöffnete Eingabedatei
/// @return Die Funktion gibt einen Pointer auf den ersten Token zurück
/// @attention Die übergebene Eingabedatei muss bereits geöffnet sein!
treenode_t *parse(void){
	// Lexer aufrufen -> Tokenstream
	token = readTokensFromFile(src_file);

	printf("\nParsing...\n\n");

	//expectTokenType(tok_bofeof, "Dateianfang erwartet");
	//next();
	treenode_t *root = program();
	//expectTokenType(tok_bofeof, "Dateiende erwartet");

	return root;
}

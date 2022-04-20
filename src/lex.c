// ToDo
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "types.h"
#include "turtle.h"

#define MAX_TOKEN_LENGTH 128

int row, col;
static srcpos_t tok_pos;     
static srcpos_t prev_tok_pos;     

type_t getTokenType(char *tok){

	if (tok[0] == '^') return oper_pow;
	if (tok[0] == '*') return oper_mul;
	if (tok[0] == '/') return oper_div;
	if (tok[0] == '+') return oper_add;
	if (tok[0] == '|') return oper_abs;
	if (tok[0] == '(') return oper_lpar;
	if (tok[0] == ')') return oper_rpar;
	if (tok[0] == ',') return oper_sep;
	if (tok[0] == '=') return oper_equ;
	if (tok[0] == '<') {
		if (tok[1] == '>') return oper_nequ;
		else if (tok[1] == '=') return oper_lequ;
		else return oper_less;
	}
	if (tok[0] == '>') {
		if (tok[1] == '=') return oper_gequ;
		else return oper_grtr;
	}

	if (tok[0] == '.' || isdigit(tok[0])) {
		int tok_len = sizeof tok / sizeof tok[0];
		bool isDot = (tok[0] == '.') ? true : false;
		for (int i = 1; i < tok_len-1; i++) {
			if (isdigit(tok[i]) || (tok[i] == '.' && !isDot)) {
				if (tok[i] == '.') {
					isDot = true;
				}
				continue;
			}
			else {
				fprintf(stderr, "Unzulässiger Zahlenwert in Zeile %d, Spalte %d\n", row, col);
				exit(EXIT_FAILURE);
			}
		}
		return oper_const;
	}
	

	// Namenstabelle prüfen
	for (int i = 0; i < nameCount; i++) {
		if (strcmp(tok, name_tab[i].name) == 0) return name_tab[i].type;
	}

	//prüfen, ob zulässiger Variablen- oder Funktionsname, um es Namenstabelle hinzuzufügen
	if (tok[0] == '_' || isalpha(tok[0]) || tok[0] == '@') {
			for(int i = 0; tok[i] != '\0'; i++){
				if(isalpha(tok[i]) || isdigit(tok[i]) || tok[i] == '_' || tok[0] == '@') {
					continue;
				}
				else {
					//kein zulässiger Variablen- oder Funktionsname -> Fehlermeldung & Abbruch
					fprintf(stderr, "Unzulässiger Variablen- oder Funktionsname in Zeile %d, Spalte %d\n", row, col);
					exit(EXIT_FAILURE);
				}
			}

			if (nameCount <= MAX_NAMES) {
					fprintf(stderr, "Zu viele Variablen- und Funktionsnamen\n");
					exit(EXIT_FAILURE);				
			}
		type_t type = (tok[0] == '@') ? name_glob : name_any;
		printf("type %d", type);
		nameentry_t *name_entry = name_tab[nameCount];
		name_entry->type = type;
		name_entry->name = tok; // ToDo: create Copy of string (tok is not persistent!)
		nameCount++;
		return type;
	} 

	printf("Type not found: %s\n", tok);

}

token_t *firstTok;
token_t *currentTok;

void initTokenStream(){
	firstTok = currentTok = malloc(sizeof(token_t));
	firstTok->tok = (char *) (firstTok->prev = firstTok->next = NULL);
}

void addToken(char *tok, type_t type){
	// some logging
	printf("Token: %s, Col %d, Line %d \n", tok, col, row);
	// Create new token
	token_t *tokPtr = firstTok->tok == NULL ? firstTok : malloc(sizeof(token_t));
	// Create own copy of token string
	char *tokCopy = malloc(strlen(tok) + 1);
	strcpy(tokCopy, tok);
	// save data to token
	tokPtr->tok = tokCopy;
	tokPtr->type = type;
	tokPtr->pos.col = col;
	tokPtr->pos.line = row;
	// move pointers around
	currentTok->next = tokPtr;
	tokPtr->prev = currentTok;
	currentTok = tokPtr;
}

void revertToken(){
	currentTok = currentTok->prev;
	free(currentTok->next);
	currentTok->next = NULL;
}

bool isSpecial(char c){
	return c == '(' || c == ')' || c == ',' || c =='+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '|' || c == '=';
}

token_t *readTokensFromFile(FILE *file){
	row = col = 1;
	// Buffer for current token
	char *buf = malloc(MAX_TOKEN_LENGTH * sizeof(char));
	// last read char
	char c;
	char lastC = '\n';
	// length of current token
	int len = 0;
	// init token stream
	initTokenStream();
	// add bof to token stream
	addToken("\0", tok_bofeof);
	// read char by char
	while(lastC != EOF && (c = fgetc(file)) != EOF){
		if(c == ' ' || c == '\t'){
			// wird mit dem Whitespace ein Token abgeschlossen?
			if(len > 0){
				buf[len] = '\0';
				type_t type = getTokenType((char *) buf);
				addToken(buf, type);
				// Spalte aktualisieren
				col += len;
				len = 0;
			}
			// Whitespaces werden ignoriert
			col++;
			continue;
		}else if (isSpecial(c)){
			// wird mit dem Sonderzeichen ein Token abgeschlossen?
			if(len > 0){
				buf[len] = '\0';
				type_t type = getTokenType((char *) buf);
				addToken(buf, type);
				// Spalte aktualisieren
				col += len;
				len = 0;
			}
			// das Sonderzeichen selbst ist ein Token und muss behandelt werden

			// kombinierte Sonderzeichen ("<=", ">=") werden zu einem Token zusammengefasst
			// ToDo: gibt es noch mehr zusammengesetzte Operatoren?
			if((lastC == '>' || lastC == '<') && c == '='){
				revertToken(); //ToDo
				// Buffer befüllen
				buf[0] = lastC;
				buf[1] = c;
				buf[2] = '\0';
				// <>=
				type_t type = getTokenType((char *) buf);
				addToken(buf, type);
				// Spalte aktualisieren
				col++;
			}

			// Sonderzeichen als Token hinzufügen
			buf[0] = c;
			buf[1] = '\0';
			type_t type = getTokenType((char *) buf);
			addToken(buf, type);

			// Spalte aktualisieren
			col++;
		}else if (c == '\"' && lastC == '\n'){ // Kommentare werden ignoriert
			while((c = fgetc(file)) != '\n' && c != EOF);
			lastC = c;
			row++; col=0;
		}else if(c == '\n'){ // aktuelle Zeile ist beendet
			// Falls nötig, schließe den Token vor dem newline ab
			if(len > 0){
				buf[len] = '\0';
				type_t type = getTokenType((char *)buf);
				addToken(buf, type);
			}
			// Zeile, Spalte und Länge zurücksetzen
			row++; col = 0; len = 0;
		}else{
			// Spalte für nächsten Durchlauf aktualisieren
			buf[len++] = c;
		}
		lastC = c;
	}
	// remaining token
	if(len > 0){
		buf[len] = '\0';
		type_t type = getTokenType((char *) buf);
		addToken(buf, type);
	}

	addToken("\0", tok_bofeof);

	// ToDo: einzelne Wörter auslesen -> Beispiel: "path circle(r,n)" wird zu "path|circle|(|r|,|n|)"
	// ToDo: Wörter Typen zuordnen -> siehe getTokenType
	// ToDo: Tokenliste verketten und zurückgeben
	free(buf);
	return firstTok;
}

// ToDo
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "types.h"
#include "turtle.h"

#define MAX_TOKEN_LENGTH 128

int row, col;

type_t getTokenType(char *tok){
	// ToDo: const numbers
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

	if(isdigit(tok[0])) return oper_const;
	
	// Namenstabelle prüfen
	for (int i = 0; i < nameCount; i++) {
		if (strcmp(tok, name_tab[i].name) == 0) return name_tab[i].type;
	}
	printf("Type not found: %s\n", tok);
	// ToDo: ggf. Namenstabelle hinzufügen
//	return NULL;
}

token_t *firstTok;
token_t *currentTok;

void initTokenStream(){
	firstTok = currentTok = malloc(sizeof(token_t));
	firstTok->tok = (char *) (firstTok->prev = firstTok->next = NULL);
}

void addToken(char *tok, type_t type){
	// Create new token
	token_t *tokPtr = firstTok->tok == NULL ? firstTok : malloc(sizeof(token_t));
	// Create own copy of token string
	char *tokCopy = malloc(strlen(tok) + 1);
  strcpy(tokCopy, tok);
	// save data to token
	tokPtr->tok = tokCopy;
	tokPtr->type = type;
	// move pointers around
	currentTok->next = tokPtr;
	tokPtr->prev = currentTok;
	currentTok = tokPtr;
}

token_t *readTokensFromFile(FILE *file){
	row = col = 0;
	// Buffer for current token
	char *buf = malloc(MAX_TOKEN_LENGTH * sizeof(char));
	// last read char
	char c;
	// length of current token
	int len = 0;
	// Help Buffer for cutting tokens
	char *help_buf = malloc(MAX_TOKEN_LENGTH * sizeof(char) + 1);
	//Help char
	char *help_c;
  // init token stream
	initTokenStream();
	// read char by char
	while((c = fgetc(file)) != EOF){
		//delete comments
		if (c == '"') {
			while ((c = fgetc(file)) != '\n'){
			}
			buf[len] = '\0';
		}
		if (c == '(' || c == ')' || c == ',' || c =='+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '|'){
			
			help_c = &c;

			help_buf = strtok(buf, " ");
			while ( help_buf != NULL) {
				help_buf = strtok(NULL, " ");
			}
			//wenn es nicht nur eine Klammer o.ä. im Token ist, sondern auch davor mehr ohne Leerzeichen
			if (strcmp(help_buf, help_c) != 0) {
				//ersetze das Zeichen z.B. Klammer durch Endzeichen
				help_buf[strlen(help_buf)] = '\0';
				type_t type = getTokenType((char *)help_buf);
				printf("Token: %s, Type: %d\n", help_buf, type);
				addToken(help_buf, type);

				type = getTokenType(help_c);
				printf("Token: %s, Type: %d\n", help_c, type);
				addToken(help_c, type);
			}
			else {
				type_t type = getTokenType(help_c);
				printf("Token: %s, Type: %d\n", help_c, type);
				addToken(help_c, type);
			}

			buf[len] = '\0';
			buf[len+1] = c;
			buf[len+2] = '\0';
			len = 0;

		}

		if(c == '\n' || c == ' '){
			buf[len] = '\0';
			type_t type = getTokenType((char *)buf);
			printf("Token: %s, Type: %d\n", buf, type);
			addToken(buf, type);
			len = 0;
		}else{
			buf[len++] = c;
			// ToDo: check max len
		}
	}
	// add last token
	buf[len] = '\0';
	type_t type = getTokenType((char *)buf);
	addToken(buf, type);
	printf("Token: %s, Type: %d\n", buf, type);

	// ToDo: einzelne Wörter auslesen -> Beispiel: "path circle(r,n)" wird zu "path|circle|(|r|,|n|)"
	// ToDo: Wörter Typen zuordnen -> siehe getTokenType
	// ToDo: Tokenliste verketten und zurückgeben
	free(buf);
	return firstTok;
}

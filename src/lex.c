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
	// ToDo: type oper_... prüfen
	// ToDo: const numbers
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
  // init token stream
	initTokenStream();
	// read char by char
	while((c = fgetc(file)) != EOF){
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

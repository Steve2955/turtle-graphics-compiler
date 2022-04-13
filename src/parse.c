#include <stdio.h>
#include <stdlib.h>

#include "turtle.h"
#include "types.h"

token_t *token;

token_t *next() {
	return token = token->next;
}

token_t *prev() {
	return token = token->prev;
}

void expectTokenType(type_t expected, char *error){
	if(token == NULL || token->type != expected){
		fprintf(stderr, "Unerwarteter Token-Typ: %s\n", error);
		exit(EXIT_FAILURE);
	}
}

void statement();

// STATEMENTS ::= STATEMENT { STATEMENT }
void statements(){
	statement();
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
	statements();
	// "end" expected after statements
	expectTokenType(keyw_end, "\"end\" erwartet");
	next();
}

/*
Haupt-Funktion des Parser
- Lexer liefer Tokenstream
- Tokenstream wird in Syntax-Baum umgewandelt
*/

treenode_t *parse(void){
	// Lexer aufrufen -> Tokenstream
	token = readTokensFromFile(src_file);

	expectTokenType(tok_bofeof, "Dateianfang erwartet");
	next();
	treenode_t *root = program();
	expectTokenType(tok_bofeof, "Dateiende erwartet");



	treenode_t *start = NULL;
	treenode_t *last = NULL;

	while(token->next != NULL){
		treenode_t *cur = malloc(sizeof(treenode_t));
		switch (token->type){
			case keyw_walk:
				cur->type = token->type;
				cur->d.walk = keyw_walk;
				treenode_t *sonW = malloc(sizeof(treenode_t));
      			sonW->d.val = atof(token->next->tok);
				sonW->type = oper_const;
				cur->son[0] = sonW;
				token = token->next; // skip oper
				break;
			case keyw_jump:
				cur->type = token->type;
				cur->d.walk = keyw_walk;
				treenode_t *sonJ = malloc(sizeof(treenode_t));
      			sonJ->d.val = atof(token->next->tok);
				sonJ->type = oper_const;
				cur->son[0] = sonJ;
				token = token->next; // skip oper
				break;
			default:
				printf("Error: Unknown keyword: %s\n", token->tok);
				break;
		}
		// throw around some pointers
		if(last == NULL){
			start = last = cur;
		}else{
			last->next = cur;
			last = cur;
			cur->next = NULL;
		}
		// move to next token
		token = token->next;
		if(token == NULL){
			break;
		}
	}

	root = start;
	expectTokenType(tok_bofeof, "Dateiende erwartet");
	return root;
}
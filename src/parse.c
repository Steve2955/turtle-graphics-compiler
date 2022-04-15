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

treenode_t *constant(){
	treenode_t *node = malloc(sizeof(treenode_t));
	node->d.val = atof(token->tok);
	node->type = oper_const;
	next();
	return node;
}

treenode_t *expression(){
	switch(token->type){
		case oper_const:
			return constant();
	}
}

treenode_t *statement(){
	treenode_t *statement = malloc(sizeof(treenode_t));
	switch(token->type){
		case keyw_walk:
		case keyw_jump:
			statement->type = token->type;
			next();
			statement->d.walk = keyw_walk;
			statement->son[0] = expression();
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
		case keyw_color:
			statement->type = token->type;
			next();
			statement->son[0] = expression();
			expectTokenType(oper_sep, "");
			next();
			statement->son[1] = expression();
			expectTokenType(oper_sep, "");
			next();
			statement->son[2] = expression();
			return statement;
		case keyw_clear:
		case keyw_stop:
		case keyw_finish:
			statement->type = token->type;
			next();
			return statement;
	}
}

// STATEMENTS ::= STATEMENT { STATEMENT }
treenode_t *statements(){
	treenode_t *firstNode = NULL;
	treenode_t *currentNode = firstNode;
	while(token->type != keyw_end && token->type != keyw_endcalc && token->type != keyw_endpath){
		printf("Token: %s\n", token->tok);
		treenode_t *newNode = statement();
		if(newNode == NULL){
			fprintf(stderr, "Fehler beim Parsen des aktuellen Statements");
			exit(EXIT_FAILURE);
		}
		if(firstNode == NULL){
			currentNode = firstNode = newNode;
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

/*
Haupt-Funktion des Parser
- Lexer liefer Tokenstream
- Tokenstream wird in Syntax-Baum umgewandelt
*/

treenode_t *parse(void){
	// Lexer aufrufen -> Tokenstream
	token = readTokensFromFile(src_file);

	//expectTokenType(tok_bofeof, "Dateianfang erwartet");
	//next();
	treenode_t *root = program();
	//expectTokenType(tok_bofeof, "Dateiende erwartet");

	return root;
}
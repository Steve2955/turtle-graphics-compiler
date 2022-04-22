#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "turtle.h"
#include "types.h"

treenode_t *expression();

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

nameentry_t *findNameEntry(char *name){
	for(int i = 0; i < nameCount; i++){
		if(strcmp(name, name_tab[i].name) == 0){
			return &name_tab[i];
		}
	}
	return NULL;
}

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

// OPERAND ::= [ "-" ] ( "sqrt" | "sin" | "cos" | "tan" "(" EXPR ")" | "(" EXPR ")" | "|" EXPR "|" | "[" EXPR "]" | ZIFFER {ZIFFER} ["." {ZIFFER}] | VAR )
treenode_t *operand(){
	treenode_t *a = malloc(sizeof(treenode_t));
	switch(token->type){
		case oper_const:
			a->d.val = atof(token->tok);
			a->type = token->type;
			next();
			return a;
		case name_any:
			printf("any name\n");
			break;
		case name_glob:
			printf("global name\n");
			break;
		case name_math_sin:
			a->type = oper_lpar;
			a->d.p_name = findNameEntry(token->tok);
			//a->d.p_name. =
			next();
			a = args(a);
			return a;
	}
	printf("unknown type: %d\n", token->type);
}

// FAKTOR ::= OPERAND [ "^" FAKTOR ]
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

/// TERM ::= FAKTOR(a) { ( "*" | "/" ) FAKTOR(b) }
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


// EXPR ::= TERM(a) { ( "-" | "+" ) TERM(b) }
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

	printf("\nParsing...\n\n");

	//expectTokenType(tok_bofeof, "Dateianfang erwartet");
	//next();
	treenode_t *root = program();
	//expectTokenType(tok_bofeof, "Dateiende erwartet");

	return root;
}
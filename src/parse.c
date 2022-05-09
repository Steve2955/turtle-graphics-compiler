#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "turtle.h"
#include "types.h"

treenode_t *expression();
treenode_t *statements();

token_t *token;


token_t *next() {
	if(token->next != NULL) printf("Token: %s\n", token->next->tok);
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

nameentry_t *findVarName(void){
	nameentry_t *v = findNameEntry(token->tok);
	// ToDo check for var type
	return v;
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
			statement->d.p_name = findVarName();
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
			statement->d.p_name = findVarName();
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
	}
}

// STATEMENTS ::= STATEMENT { STATEMENT }
treenode_t *statements(){
	treenode_t *firstNode = NULL;
	treenode_t *currentNode = firstNode;
	while(token->type != keyw_end && token->type != keyw_endcalc && token->type != keyw_endpath  && token->type != keyw_done){
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
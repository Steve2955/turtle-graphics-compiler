#include <stdio.h>
#include <stdlib.h>

#include "turtle.h"
#include "types.h"

/*
Haupt-Funktion des Parser
- Lexer liefer Tokenstream
- Tokenstream wird in Syntax-Baum umgewandelt
*/
treenode_t *parse(void){
	token_t *token = readTokensFromFile(src_file);

	treenode_t *root = NULL;
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
	return root;
}
#include "turtle.h"
#include "types.h"

/*
Haupt-Funktion des Parser
- Lexer liefer Tokenstream
- Tokenstream wird in Syntax-Baum umgewandelt
*/
treenode_t *parse(void){
    token_t *tokens = readTokensFromFile(src_file);
    // Pfad und
}
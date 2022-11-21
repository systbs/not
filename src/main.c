#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "memory.h"
#include "token.h"
#include "lexer.h"
#include "list.h"
#include "node.h"
#include "parser.h"
#include "semantic.h"

int
main(int argc, char **argv)
{
	argc--;
    argv++;

    // parse arguments
    if (argc < 1) {
        fprintf(stderr, "usage: file ...\n");
        return -1;
    }
    
    char *path = *argv;

    argc--;
    argv++;
    
    lexer_t *lexer;
    if(!(lexer = lexer_create(path, lexer_load_file(path)))){
    	fprintf(stderr, "could not create lexer\n");
    	return -1;
    }
    
    parser_t *parser;
    if(!(parser = parser_create(lexer))){
    	fprintf(stderr, "could not create parser\n");
    	return -1;
    }
    
    node_t *node;
    node = parser_module(parser);
    if(!node){
    	fprintf(stderr, "the compiler has stopped with an error\n");
    	return -1;
    }
    
    return 0;
}

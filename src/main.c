#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "types/types.h"
#include "container/list.h"
#include "token/position.h"
#include "token/token.h"
#include "program.h"
#include "scanner/file.h"
#include "scanner/scanner.h"
#include "ast/node.h"
#include "parser/parser.h"
#include "parser/error.h"
#include "parser/syntax.h"
#include "parser/symbol.h"
#include "parser/graph.h"
#include "utils/utils.h"
#include "utils/path.h"

int
main(int argc, char **argv)
{
	argc--;
    argv++;

    // parse arguments

    
    char *path = "./test/test.q";

    argc--;
    argv++;

    program_t program;

    path_get_current_directory(program.base_path, sizeof(program.base_path));
    if(path_is_relative(path))
    {
        path_join(program.base_path, path, program.base_file, sizeof(program.base_file));
    }
    else {
        strcpy(program.base_file, path);
    }

    list_t *errors;
    errors = list_create();
    if(!errors)
    {
        return -1;
    }
    
    parser_t *parser;
    parser = parser_create(&program, program.base_file, errors);
    if(!parser)
    {
    	return -1;
    }
    
    node_t *root;
    root = parser_module(parser);
    if(!root)
    {
        if(list_count(errors) > 0)
        {
            goto print_error;
        }
    	return -1;
    }

    
    syntax_t *syntax;
    syntax = syntax_create(&program, errors);
    if(!syntax)
    {
        return -1;
    }

    int32_t syntax_result;
    syntax_result = syntax_run(syntax, root);
    if(!syntax_result)
    {
        if(list_count(errors) > 0)
        {
            goto print_error;
        }
    	return -1;
    }

    graph_t *graph;
    graph = graph_create(&program, errors);
    if(!graph)
    {
        return -1;
    }

    int32_t graph_result;
    graph_result = graph_run(graph, syntax);
    if(!graph_result)
    {
        if(list_count(errors) > 0)
        {
            goto print_error;
        }
    	return -1;
    }

    return 0;

    ilist_t *a;
    print_error:
    for(a = errors->begin; a != errors->end; a = a->next)
    {
        error_t *error;
        error = (error_t *)a->value;

        char relative_path[_MAX_DIR + _MAX_FNAME + _MAX_EXT];
        path_get_relative(program.base_path, error->position.path, relative_path, sizeof(relative_path));
        
        fprintf(stderr, 
            "%s-%lld:%lld:error:%s\n", 
            relative_path, 
            error->position.line, 
            error->position.column, 
            error->message
        );
    }

    return -2;
}

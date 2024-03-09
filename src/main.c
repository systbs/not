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

    char *base_path = malloc(_MAX_DIR + _MAX_FNAME + _MAX_EXT);
    memset(base_path, 0, _MAX_DIR + _MAX_FNAME + _MAX_EXT);

    char *base_file = malloc(_MAX_DIR + _MAX_FNAME + _MAX_EXT);
    memset(base_file, 0, _MAX_DIR + _MAX_FNAME + _MAX_EXT);

    if (path_is_root(path))
    {
			path_normalize(getenv ("QALAM-PATH"), base_path, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
			path_join(base_path, path + 2, base_file, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
    }
    else
    {
			path_get_current_directory(base_path, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
			if(path_is_relative(path))
			{
				path_join(base_path, path, base_file, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
			}
			else 
			{
				path_normalize(path, base_file, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
			}
    }

    program.errors = list_create();
    if(!program.errors)
    {
      return -1;
    }

		program.imports = list_create();
    if(!program.imports)
    {
      return -1;
    }
    
    parser_t *parser;
    parser = parser_create(&program, base_file);
    if(!parser)
    {
    	return -1;
    }
    
    node_t *node;
    node = parser_module(&program, parser);
    if(!node)
    {
			if(list_count(program.errors) > 0)
			{
				goto print_error;
			}
    	return -1;
    }

    int32_t result;
    result = syntax_run(&program, node);
    if(result == -1)
    {
        if(list_count(program.errors) > 0)
        {
            goto print_error;
        }
    	return -1;
    }

    return 0;

    ilist_t *a;
    print_error:
    for(a = program.errors->begin; a != program.errors->end; a = a->next)
    {
        error_t *error;
        error = (error_t *)a->value;

        char relative_path[_MAX_DIR + _MAX_FNAME + _MAX_EXT];
        path_get_relative(base_path, error->position.path, relative_path, sizeof(relative_path));
        
        fprintf(stderr, 
            "%s-%lld:%lld:error:%s\n", 
            relative_path, 
            error->position.line, 
            error->position.column, 
            error->message
        );
    }

    return -1;
}

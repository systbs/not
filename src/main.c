#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "types/types.h"
#include "container/list.h"
#include "token/position.h"
#include "token/token.h"
#include "program.h"
#include "scanner/file.h"
#include "scanner/scanner.h"
#include "ast/node.h"
#include "parser/syntax.h"
#include "parser/error.h"
#include "parser/semantic.h"
#include "utils/utils.h"
#include "utils/path.h"


int
main(int argc, char **argv)
{
	program_t *program = program_create();
	if (program == NULL)
	{
		return -1;
	}

	int32_t i;
	for (i = 1;i < argc; i++)
	{
		if (strcmp(argv[i], "-f") == 0)
		{
			i += 1;
			program_resolve(program, argv[i]);
		}
	}

	if (strcmp(program->base_path, "") == 0)
	{
		fprintf(stderr, 
		"qalam: fatal: no input file specified\n"
		"using:qalam -f [file] ...\n"
		"type qalam -h for more help.\n");
		return 0;
	}
	
	syntax_t *syntax = syntax_create(program, program->base_file);
	if(syntax == NULL)
	{
		return -1;
	}
	
	node_t *node = syntax_module(program, syntax);
	if(node == NULL)
	{
		if(list_count(program->errors) > 0)
		{
			goto region_report;
		}
		return -1;
	}

	int32_t result = semantic_run(program, node);
	if(result == -1)
	{
		if(list_count(program->errors) > 0)
		{
			goto region_report;
		}
		return -1;
	}

	return 0;

	region_report:
	program_report(program);

	return -1;
}

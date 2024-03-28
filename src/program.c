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
#include "parser/syntax.h"
#include "parser/error.h"
#include "parser/semantic.h"
#include "utils/utils.h"
#include "utils/path.h"

void
program_report(program_t *program)
{
  ilist_t *a;
  for(a = program->errors->begin; a != program->errors->end; a = a->next)
  {
		error_t *error = (error_t *)a->value;

		char relative_path[MAX_URI];
		path_get_relative(program->base_path, error->position.path, relative_path, sizeof(relative_path));
		
		fprintf(stderr, 
				"%s-%lld:%lld:error:%s\n", 
				relative_path, 
				error->position.line, 
				error->position.column, 
				error->message
		);
  }
}

void_t
program_resolve(program_t *program, char *path)
{
	if (path_is_root(path))
  {
    path_normalize(getenv ("QALAM-PATH"), program->base_path, MAX_URI);
    path_join(program->base_path, path + 2, program->base_file, MAX_URI);
  }
  else
  {
    path_get_current_directory(program->base_path, MAX_URI);
    if(path_is_relative(path))
    {
      path_join(program->base_path, path, program->base_file, MAX_URI);
    }
    else 
    {
      path_normalize(path, program->base_file, MAX_URI);
    }
  }
}

program_t *
program_create()
{
	program_t *program = (program_t *)malloc(sizeof(program_t));
	if (program == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(program_t));
		return NULL;
	}
	memset(program, 0, sizeof(program_t));

	program->base_path = malloc(MAX_URI);
	if (program->base_path == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %d bytes\n", MAX_URI);
		return NULL;
	}
  memset(program->base_path, 0, MAX_URI);

	program->base_file = malloc(MAX_URI);
	if (program->base_file == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %d bytes\n", MAX_URI);
		return NULL;
	}
  memset(program->base_file, 0, MAX_URI);

	program->errors = list_create();
	if(program->errors == NULL)
	{
		return NULL;
	}

	program->modules = list_create();
	if(program->modules == NULL)
	{
		return NULL;
	}

	program->stack = list_create();
	if(program->stack == NULL)
	{
		return NULL;
	}

	return program;
}
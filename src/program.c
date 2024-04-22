#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "types/types.h"
#include "utils/utils.h"
#include "utils/path.h"
#include "container/list.h"
#include "token/position.h"
#include "token/token.h"
#include "program.h"
#include "scanner/scanner.h"
#include "ast/node.h"
#include "parser/error.h"
#include "parser/syntax/syntax.h"
#include "parser/semantic/semantic.h"


void_t
program_report(program_t *program)
{
  ilist_t *a;
  for(a = program->errors->begin; a != program->errors->end; a = a->next)
  {
		error_t *error = (error_t *)a->value;

		char base_path[MAX_URI];
		path_get_current_directory(base_path, MAX_URI);

		char relative_path[MAX_URI];
		path_get_relative(base_path, error->position.path, relative_path, sizeof(relative_path));
		
		fprintf(stderr, 
				"%s-%lld:%lld:error:%s\n", 
				relative_path, 
				error->position.line, 
				error->position.column, 
				error->message
		);

		FILE *fp = fopen(error->position.path, "rb");

		if (fp == NULL)
		{
			fprintf(stderr, "%s-%u:could not open(%s)\n", 
				__FILE__, __LINE__, error->position.path);
			return;
		}
		
		uint64_t line = 0;

		char chunk[128];
		size_t len = sizeof(chunk);

		char *buf = malloc(len);
		if (buf == NULL)
		{
			fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n", 
				__FILE__, __LINE__, sizeof(chunk));
			return;
		}

		buf[0] = '\0';
		while(fgets(chunk, sizeof(chunk), fp) != NULL) {
			size_t len_used = strlen(buf);
			size_t chunk_used = strlen(chunk);
	
			if(len - len_used < chunk_used) {
				len *= 2;
				if((buf = realloc(buf, len)) == NULL) {
					fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n", 
						__FILE__, __LINE__, sizeof(len));
					free(buf);
					return;
				}
			}

			strncpy(buf + len_used, chunk, len - len_used);
			len_used += chunk_used;

			if(buf[len_used - 1] == '\n') {
				line += 1;

				if ((line > error->position.line - 2) && (line < error->position.line + 2))
				{
					if (line == error->position.line)
					{
						fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

						uint64_t j = 0;
						for (uint64_t i = 0;i < strlen(buf);i++)
						{
							j += 1;
							if (buf[i] == '\t')
							{
								j += 3;
							}
							
							if ((j >= error->position.column) && (j < error->position.column + error->position.length))
							{
								fprintf(stderr, "\033[1;31m%c\033[m", buf[i]);
							}
							else
							{
								fprintf(stderr, "%c", buf[i]);
							}
						}
					}
					else
					{
						fprintf(stderr, "  %lld\t|%s", line, buf);
					}
				}
				buf[0] = '\0';
			}
		}

		fclose(fp);

		free(buf);
  }
}

void_t
program_resolve(program_t *program, char *path)
{
	if (path_is_root(path))
	{
		char base_path[MAX_URI];
		path_normalize(getenv ("QALAM_PATH"), base_path, MAX_URI);
		path_join(base_path, path + 2, program->base_file, MAX_URI);
	}
	else
	{
		char base_path[MAX_URI];
		path_get_current_directory(base_path, MAX_URI);
		if(path_is_relative(path))
		{
			path_join(base_path, path, program->base_file, MAX_URI);
		}
		else 
		{
			path_normalize(path, program->base_file, MAX_URI);
		}
	}
}

node_t *
program_load(program_t *program, char *path)
{
	char *base_file = malloc(MAX_URI);
	if (base_file == NULL)
	{
		fprintf(stderr, "%s-(%u):Unable to allocted a block of %d bytes\n", 
			__FILE__, __LINE__, MAX_URI);
		return NULL;
	}
	memset(base_file, 0, MAX_URI);

	if (path_is_root(path))
	{
		char base_path[MAX_URI];
		path_normalize(getenv ("QALAM_PATH"), base_path, MAX_URI);
		path_join(base_path, path + 2, base_file, MAX_URI);
	}
	else
	{
		char base_path[MAX_URI];
		path_get_current_directory(base_path, MAX_URI);
		if(path_is_relative(path))
		{
			path_join(base_path, path, base_file, MAX_URI);
		}
		else 
		{
			path_normalize(path, base_file, MAX_URI);
		}
	}

	node_t *module1 = NULL;

	ilist_t *b1;
	for (b1 = program->modules->begin;b1 != program->modules->end;b1 = b1->next)
	{
		pair_t *pair1 = (pair_t *)b1->value;
		if (strcmp(pair1->key, base_file) == 0)
		{
			module1 = (node_t *)pair1->value;
			break;
		}
	}

	if (module1 == NULL)
	{
		syntax_t *syntax1 = syntax_create(program, base_file);
		if(syntax1 == NULL)
		{
			return NULL;
		}
		
		node_t *node1 = syntax_module(program, syntax1);
		if(node1 == NULL)
		{
			return NULL;
		}

		pair_t *pair1 = (pair_t *)malloc(sizeof(pair_t));
		if (pair1 == NULL)
		{
			fprintf(stderr, "%s-(%u):Unable to allocted a block of %zu bytes\n", 
				__FILE__, __LINE__, sizeof(program_t));
			return NULL;
		}

		pair1->key = base_file;
		pair1->value = node1;

		ilist_t *r1 = list_rpush(program->modules, pair1);
		if (r1 == NULL)
		{
			return NULL;
		}

		int32_t r2 = semantic_module(program, node1);
		if(r2 == -1)
		{
			return NULL;
		}

		module1 = node1;
	}

	return module1;
}

void_t
program_outfile(program_t *program, char *path)
{
	memcpy(program->out_file, path, strlen(path));
}

program_t *
program_create()
{
	program_t *program = (program_t *)malloc(sizeof(program_t));
	if (program == NULL)
	{
		fprintf(stderr, "%s-(%u):Unable to allocted a block of %zu bytes\n", 
			__FILE__, __LINE__, sizeof(program_t));
		return NULL;
	}
	memset(program, 0, sizeof(program_t));

	program->base_path = malloc(MAX_URI);
	if (program->base_path == NULL)
	{
		fprintf(stderr, "%s-(%u):Unable to allocted a block of %d bytes\n", 
			__FILE__, __LINE__, MAX_URI);
		return NULL;
	}
  	memset(program->base_path, 0, MAX_URI);

	program->base_file = malloc(MAX_URI);
	if (program->base_file == NULL)
	{
		fprintf(stderr, "%s-(%u):Unable to allocted a block of %d bytes\n", 
			__FILE__, __LINE__, MAX_URI);
		return NULL;
	}
  	memset(program->base_file, 0, MAX_URI);

	program->out_file = malloc(MAX_URI);
	if (program->out_file == NULL)
	{
		fprintf(stderr, "%s-(%u):Unable to allocted a block of %d bytes\n", 
			__FILE__, __LINE__, MAX_URI);
		return NULL;
	}
  	memset(program->out_file, 0, MAX_URI);

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

	program->repository = list_create();
	if(program->repository == NULL)
	{
		return NULL;
	}

	return program;
}
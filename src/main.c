#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <gmp.h>
#include <math.h>
#include <float.h>
#include <jansson.h>
#include <fcntl.h>

#include "types/types.h"
#include "utils/utils.h"
#include "utils/path.h"
#include "container/queue.h"
#include "token/position.h"
#include "token/token.h"
#include "error.h"
#include "mutex.h"
#include "config.h"
#include "repository.h"
#include "interpreter.h"
#include "thread.h"
#include "scanner/scanner.h"
#include "ast/node.h"
#include "parser/syntax/syntax.h"
#include "parser/semantic/semantic.h"
#include "interpreter/record.h"
#include "interpreter/symbol_table.h"
#include "interpreter/strip.h"
#include "interpreter/execute.h"

int main(int argc, char **argv)
{
	mpf_set_default_prec(256);

	if (not_config_init() < 0)
	{
		return -1;
	}

	if (not_repository_init() < 0)
	{
		return -1;
	}

	if (not_thread_init() < 0)
	{
		return -1;
	}

	int32_t i;
	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-f") == 0)
		{
			i += 1;
			if (not_config_set_input_file(argv[i]) < 0)
			{
				return -1;
			}
		}
	}

	if (strcmp(not_config_get_input_file(), "") == 0)
	{
		fprintf(stderr,
				"not-lang: fatal: no input file specified\n"
				"using:not -f [file] ...\n");
		return 0;
	}

	not_config_expection_set(1);

	if (not_symbol_table_init() < 0)
	{
		goto region_error;
	}

	char directory_path[MAX_PATH];
	not_path_get_current_directory(directory_path, MAX_PATH);

	if (NOT_PTR_ERROR == not_repository_load(directory_path, not_config_get_input_file()))
	{
		goto region_error;
	}

	if (not_symbol_table_destroy() < 0)
	{
		goto region_error;
	}

	not_thread_destroy();
	not_repository_destroy();

	return 0;

region_error:

	not_thread_t *t = not_thread_get_current();

	for (not_queue_entry_t *a = t->interpreter->expections->begin, *b = NOT_PTR_NULL; a != t->interpreter->expections->end; a = b)
	{
		b = a->next;
		not_record_t *expection = (not_record_t *)a->value;

		printf("%s\n", not_record_to_string(expection, ""));

		not_queue_unlink(t->interpreter->expections, a);
	}

	exit(-1);
}

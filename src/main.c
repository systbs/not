#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <gmp.h>

#include "types/types.h"
#include "utils/utils.h"
#include "utils/path.h"
#include "container/queue.h"
#include "token/position.h"
#include "token/token.h"
#include "error.h"
#include "mutex.h"
#include "config.h"
#include "module.h"
#include "interpreter.h"
#include "thread.h"
#include "scanner/scanner.h"
#include "ast/node.h"
#include "parser/syntax/syntax.h"
#include "parser/semantic/semantic.h"
#include "interpreter/record.h"
#include "interpreter/garbage.h"
#include "interpreter/symbol_table.h"
#include "interpreter/strip.h"
#include "interpreter/execute/execute.h"

int
main(int argc, char **argv)
{
	if (sy_config_init() < 0)
	{
		return -1;
	}

	if (sy_module_init() < 0)
	{
		return -1;
	}

	if (sy_thread_init() < 0)
	{
		return -1;
	}
	
	char *env = getenv (ENV_LIBRARY_KEY);
	if (env == NULL)
	{
		setenv(ENV_LIBRARY_KEY, DEFAULT_LIBRARY_PATH, 1);
	}

	int32_t i;
	for (i = 1;i < argc; i++)
	{
		if (strcmp(argv[i], "-f") == 0)
		{
			i += 1;
			if (sy_config_set_input_file(argv[i]) < 0)
			{
				return -1;
			}
		}
	}

	if (strcmp(sy_config_get_input_file(), "") == 0)
	{
		fprintf(stderr, 
		"syntax-lang: fatal: no input file specified\n"
		"using:syntax -f [file] ...\n"
		"type syntax -h for more help.\n");
		return 0;
	}

	if (sy_garbage_init() < 0)
	{
		return -1;
	}

	if (sy_symbol_table_init() < 0)
	{
		return -1;
	}

	sy_grabage_thread_data_t data;
	data.ret = 0;
	sy_thread_t *thread = sy_thread_create(sy_garbage_clean_by_thread, &data);
	if (thread == ERROR)
	{
		sy_error_system("garbage thread not created\n");
		return -1;
	}

	sy_module_entry_t *module_entry = sy_module_load(sy_config_get_input_file());
    if (module_entry == ERROR)
    {
		return -1;
	}

	if (sy_symbol_table_destroy() < 0)
	{
		return -1;
	}

	if (sy_garbage_destroy() < 0)
	{
		return -1;
	}

	if (sy_thread_destroy() < 0)
	{
		return -1;
	}

	if (sy_config_destroy() < 0)
	{
		return -1;
	}

	return 0;
}


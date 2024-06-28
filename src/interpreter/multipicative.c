#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdint.h>
#include <float.h>
#include <jansson.h>
#include <ffi.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../error.h"
#include "../mutex.h"
#include "../memory.h"
#include "../config.h"
#include "../scanner/scanner.h"
#include "../parser/syntax/syntax.h"
#include "record.h"
#include "../repository.h"
#include "../interpreter.h"
#include "../thread.h"
#include "symbol_table.h"
#include "strip.h"
#include "entry.h"
#include "helper.h"
#include "execute.h"

void mpf_mod(mpf_t result, const mpf_t a, const mpf_t b)
{
	mpf_t temp_div, temp_int;

	mpf_init(temp_div);
	mpf_init(temp_int);

	//  temp_div = a / b
	mpf_div(temp_div, a, b);

	//  temp_int = floor(temp_div)
	mpf_floor(temp_int, temp_div);

	//  result = a - (temp_int * b)
	mpf_mul(temp_int, temp_int, b); // temp_int = floor(a / b) * b
	mpf_sub(result, a, temp_int);	// result = a - floor(a / b) * b

	mpf_clear(temp_div);
	mpf_clear(temp_int);
}

void mpf_fdiv_q(mpf_t result, const mpf_t a, const mpf_t b)
{
	mpf_t temp;
	mpz_t z_result;

	mpf_init(temp);
	mpz_init(z_result);

	mpf_div(temp, a, b);

	mpz_set_f(z_result, temp);

	mpf_set_z(result, z_result);

	mpf_clear(temp);
	mpz_clear(z_result);
}

not_record_t *
not_multipicative_mul(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
	if (left->null || left->undefined || left->nan)
	{
		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_INT)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set(num1, (*(mpz_t *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_mul(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			mpf_t num1, result;
			mpf_init(result);
			mpf_init(num1);

			mpf_set_z(num1, (*(mpz_t *)(left->value)));

			mpf_mul(result, num1, (*(mpf_t *)(right->value)));
			mpf_clear(num1);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set(num1, (*(mpz_t *)(left->value)));
			mpz_init_set_si(num2, (*(char *)(right->value)));

			mpz_mul(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_FLOAT)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			mpz_t num1, num2, result;
			mpz_init(result);
			mpz_init(num1);

			mpz_set_f(num1, (*(mpf_t *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_mul(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			mpf_t result;
			mpf_init(result);
			mpf_mul(result, (*(mpf_t *)(left->value)), (*(mpf_t *)(right->value)));
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);

			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			mpf_t num2, result;
			mpf_init(result);

			mpf_init_set_si(num2, (*(char *)(right->value)));

			mpf_mul(result, (*(mpf_t *)(left->value)), num2);
			mpf_clear(num2);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_CHAR)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set_si(num1, (*(char *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_mul(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			mpf_t num1, result;
			mpf_init(result);

			mpf_init_set_si(num1, (*(char *)(left->value)));

			mpf_mul(result, num1, (*(mpf_t *)(right->value)));
			mpf_clear(num1);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set_si(num1, (*(char *)(left->value)));
			mpz_init_set_si(num2, (*(char *)(right->value)));

			mpz_mul(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_STRING)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			size_t length = strlen((char *)(left->value));
			char *str = not_memory_calloc(1, sizeof(char));
			if (!str)
			{
				not_error_no_memory();
				return NOT_PTR_ERROR;
			}

			str[0] = '\0';

			mpz_t total;
			mpz_init_set_ui(total, 1);
			mpz_mul(total, total, *(mpz_t *)(right->value));

			mpz_t size;
			mpz_init_set_ui(size, 0);
			while (mpz_cmp(size, total) < 0)
			{
				char *buf = not_memory_calloc(strlen(str) + length + 1, sizeof(char));
				if (!buf)
				{
					not_error_no_memory();
					not_memory_free(str);
					mpz_clear(size);
					mpz_clear(total);
					return NOT_PTR_ERROR;
				}
				sprintf(buf, "%s%s", str, (char *)(left->value));
				not_memory_free(str);
				str = buf;
				mpz_add_ui(size, size, length);
			}
			mpz_clear(size);
			mpz_clear(total);

			not_record_t *record = not_record_make_string(str);
			not_memory_free(str);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_STRUCT)
	{
		return not_call_operator_by_one_arg(node, left, right, "*", applicant);
	}

	return not_record_make_nan();
}

not_record_t *
not_multipicative_div(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
	if (left->null || left->undefined || left->nan)
	{
		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_INT)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			if (mpz_cmp_si((*(mpz_t *)(right->value)), 0) == 0)
			{
				not_error_runtime_by_node(node, "floating point exception");
				return NOT_PTR_ERROR;
			}
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set(num1, (*(mpz_t *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_div(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			if (mpf_cmp_si((*(mpf_t *)(right->value)), 0) == 0)
			{
				not_error_runtime_by_node(node, "floating point exception");
				return NOT_PTR_ERROR;
			}

			mpf_t num1, result;
			mpf_init(result);
			mpf_init(num1);

			mpf_set_z(num1, (*(mpz_t *)(left->value)));

			mpf_div(result, num1, (*(mpf_t *)(right->value)));
			mpf_clear(num1);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			if ((*(char *)(right->value)) == 0)
			{
				not_error_runtime_by_node(node, "floating point exception");
				return NOT_PTR_ERROR;
			}

			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set(num1, (*(mpz_t *)(left->value)));
			mpz_init_set_si(num2, (*(char *)(right->value)));

			mpz_div(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_FLOAT)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			if (mpz_cmp_si((*(mpz_t *)(right->value)), 0) == 0)
			{
				not_error_runtime_by_node(node, "floating point exception");
				return NOT_PTR_ERROR;
			}

			mpz_t num1, num2, result;
			mpz_init(result);
			mpz_init(num1);

			mpz_set_f(num1, (*(mpf_t *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_div(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			if (mpf_cmp_si((*(mpf_t *)(right->value)), 0) == 0)
			{
				not_error_runtime_by_node(node, "floating point exception");
				return NOT_PTR_ERROR;
			}

			mpf_t result;
			mpf_init(result);
			mpf_div(result, (*(mpf_t *)(left->value)), (*(mpf_t *)(right->value)));
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);

			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			if ((*(char *)(right->value)) == 0)
			{
				not_error_runtime_by_node(node, "floating point exception");
				return NOT_PTR_ERROR;
			}

			mpf_t num2, result;
			mpf_init(result);

			mpf_init_set_si(num2, (*(char *)(right->value)));

			mpf_div(result, (*(mpf_t *)(left->value)), num2);
			mpf_clear(num2);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_CHAR)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			if (mpz_cmp_si((*(mpz_t *)(right->value)), 0) == 0)
			{
				not_error_runtime_by_node(node, "floating point exception");
				return NOT_PTR_ERROR;
			}

			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set_si(num1, (*(char *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_div(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			if (mpf_cmp_si((*(mpf_t *)(right->value)), 0) == 0)
			{
				not_error_runtime_by_node(node, "floating point exception");
				return NOT_PTR_ERROR;
			}

			mpf_t num1, result;
			mpf_init(result);

			mpf_init_set_si(num1, (*(char *)(left->value)));

			mpf_div(result, num1, (*(mpf_t *)(right->value)));
			mpf_clear(num1);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			if ((*(char *)(right->value)) == 0)
			{
				not_error_runtime_by_node(node, "floating point exception");
				return NOT_PTR_ERROR;
			}

			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set_si(num1, (*(char *)(left->value)));
			mpz_init_set_si(num2, (*(char *)(right->value)));

			mpz_div(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_STRUCT)
	{
		return not_call_operator_by_one_arg(node, left, right, "/", applicant);
	}

	return not_record_make_nan();
}

not_record_t *
not_multipicative_mod(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
	if (left->null || left->undefined || left->nan)
	{
		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_INT)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set(num1, (*(mpz_t *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_mod(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			mpf_t num1, result;
			mpf_init(result);
			mpf_init(num1);

			mpf_set_z(num1, (*(mpz_t *)(left->value)));

			mpf_mod(result, num1, (*(mpf_t *)(right->value)));
			mpf_clear(num1);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set(num1, (*(mpz_t *)(left->value)));
			mpz_init_set_si(num2, (*(char *)(right->value)));

			mpz_mod(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_FLOAT)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			mpz_t num1, num2, result;
			mpz_init(result);
			mpz_init(num1);

			mpz_set_f(num1, (*(mpf_t *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_mod(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			mpf_t result;
			mpf_init(result);
			mpf_mod(result, (*(mpf_t *)(left->value)), (*(mpf_t *)(right->value)));
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);

			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			mpf_t num2, result;
			mpf_init(result);

			mpf_init_set_si(num2, (*(char *)(right->value)));

			mpf_mod(result, (*(mpf_t *)(left->value)), num2);
			mpf_clear(num2);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_CHAR)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set_si(num1, (*(char *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_mod(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			mpf_t num1, result;
			mpf_init(result);

			mpf_init_set_si(num1, (*(char *)(left->value)));

			mpf_mod(result, num1, (*(mpf_t *)(right->value)));
			mpf_clear(num1);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set_si(num1, (*(char *)(left->value)));
			mpz_init_set_si(num2, (*(char *)(right->value)));

			mpz_mod(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_STRUCT)
	{
		return not_call_operator_by_one_arg(node, left, right, "%", applicant);
	}

	return not_record_make_nan();
}

not_record_t *
not_multipicative_epi(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
	if (left->null || left->undefined || left->nan)
	{
		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_INT)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set(num1, (*(mpz_t *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_fdiv_q(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			mpf_t num1, result;
			mpf_init(result);
			mpf_init(num1);

			mpf_set_z(num1, (*(mpz_t *)(left->value)));

			mpf_fdiv_q(result, num1, (*(mpf_t *)(right->value)));
			mpf_clear(num1);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set(num1, (*(mpz_t *)(left->value)));
			mpz_init_set_si(num2, (*(char *)(right->value)));

			mpz_fdiv_q(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_FLOAT)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			mpz_t num1, num2, result;
			mpz_init(result);
			mpz_init(num1);

			mpz_set_f(num1, (*(mpf_t *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_fdiv_q(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			mpf_t result;
			mpf_init(result);
			mpf_fdiv_q(result, (*(mpf_t *)(left->value)), (*(mpf_t *)(right->value)));
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);

			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			mpf_t num2, result;
			mpf_init(result);

			mpf_init_set_si(num2, (*(char *)(right->value)));

			mpf_fdiv_q(result, (*(mpf_t *)(left->value)), num2);
			mpf_clear(num2);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_CHAR)
	{
		if (right->null || right->undefined || right->nan)
		{
			return not_record_make_nan();
		}
		else if (right->kind == RECORD_KIND_INT)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set_si(num1, (*(char *)(left->value)));
			mpz_init_set(num2, (*(mpz_t *)(right->value)));

			mpz_fdiv_q(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_FLOAT)
		{
			mpf_t num1, result;
			mpf_init(result);

			mpf_init_set_si(num1, (*(char *)(left->value)));

			mpf_fdiv_q(result, num1, (*(mpf_t *)(right->value)));
			mpf_clear(num1);
			not_record_t *record = not_record_make_float_from_f(result);
			mpf_clear(result);
			return record;
		}
		else if (right->kind == RECORD_KIND_CHAR)
		{
			mpz_t num1, num2, result;
			mpz_init(result);

			mpz_init_set_si(num1, (*(char *)(left->value)));
			mpz_init_set_si(num2, (*(char *)(right->value)));

			mpz_fdiv_q(result, num1, num2);
			mpz_clear(num1);
			mpz_clear(num2);
			not_record_t *record = not_record_make_int_from_z(result);
			mpz_clear(result);
			return record;
		}

		return not_record_make_nan();
	}
	else if (left->kind == RECORD_KIND_STRUCT)
	{
		return not_call_operator_by_one_arg(node, left, right, "\\", applicant);
	}

	return not_record_make_nan();
}

not_record_t *
not_multipicative(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
	if (node->kind == NODE_KIND_MUL)
	{
		not_node_binary_t *binary = (not_node_binary_t *)node->value;
		not_record_t *left = not_multipicative(binary->left, strip, applicant, origin);
		if (left == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		not_record_t *right = not_power(binary->right, strip, applicant, origin);
		if (right == NOT_PTR_ERROR)
		{
			not_record_link_decrease(left);
			return NOT_PTR_ERROR;
		}

		not_record_t *record = not_multipicative_mul(node, left, right, applicant);

		if (not_record_link_decrease(left) < 0)
		{
			return NOT_PTR_ERROR;
		}

		if (not_record_link_decrease(right) < 0)
		{
			return NOT_PTR_ERROR;
		}

		return record;
	}
	else if (node->kind == NODE_KIND_DIV)
	{
		not_node_binary_t *binary = (not_node_binary_t *)node->value;
		not_record_t *left = not_multipicative(binary->left, strip, applicant, origin);
		if (left == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		not_record_t *right = not_power(binary->right, strip, applicant, origin);
		if (right == NOT_PTR_ERROR)
		{
			not_record_link_decrease(left);
			return NOT_PTR_ERROR;
		}

		not_record_t *record = not_multipicative_div(node, left, right, applicant);

		if (not_record_link_decrease(left) < 0)
		{
			return NOT_PTR_ERROR;
		}

		if (not_record_link_decrease(right) < 0)
		{
			return NOT_PTR_ERROR;
		}

		return record;
	}
	else if (node->kind == NODE_KIND_MOD)
	{
		not_node_binary_t *binary = (not_node_binary_t *)node->value;
		not_record_t *left = not_multipicative(binary->left, strip, applicant, origin);
		if (left == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		not_record_t *right = not_power(binary->right, strip, applicant, origin);
		if (right == NOT_PTR_ERROR)
		{
			not_record_link_decrease(left);
			return NOT_PTR_ERROR;
		}

		not_record_t *record = not_multipicative_mod(node, left, right, applicant);

		if (not_record_link_decrease(left) < 0)
		{
			return NOT_PTR_ERROR;
		}

		if (not_record_link_decrease(right) < 0)
		{
			return NOT_PTR_ERROR;
		}

		return record;
	}
	else if (node->kind == NODE_KIND_EPI)
	{
		not_node_binary_t *binary = (not_node_binary_t *)node->value;
		not_record_t *left = not_multipicative(binary->left, strip, applicant, origin);
		if (left == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		not_record_t *right = not_power(binary->right, strip, applicant, origin);
		if (right == NOT_PTR_ERROR)
		{
			not_record_link_decrease(left);
			return NOT_PTR_ERROR;
		}

		not_record_t *record = not_multipicative_epi(node, left, right, applicant);

		if (not_record_link_decrease(left) < 0)
		{
			return NOT_PTR_ERROR;
		}

		if (not_record_link_decrease(right) < 0)
		{
			return NOT_PTR_ERROR;
		}

		return record;
	}
	else
	{
		return not_power(node, strip, applicant, origin);
	}
}

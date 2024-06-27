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

void cleanup_gmp_mpfr()
{
    // GMP function to free all cached memory
    mp_get_memory_functions(NULL, NULL, NULL);
    mpfr_free_cache();
}

void mpfr_to_mpf(mpf_t rop, const mpfr_t op)
{
    mpq_t q;
    mpq_init(q);
    mpfr_get_q(q, op);
    mpf_set_q(rop, q);
    mpq_clear(q);
}

void mpf_pow(mpf_t result, const mpf_t base, const mpf_t exp)
{
    mpfr_t base_fr, exp_fr, result_fr;
    mpfr_init2(base_fr, mpf_get_prec(base));
    mpfr_init2(exp_fr, mpf_get_prec(exp));
    mpfr_init2(result_fr, mpf_get_prec(result));

    mpfr_set_f(base_fr, base, MPFR_RNDN);
    mpfr_set_f(exp_fr, exp, MPFR_RNDN);

    mpfr_pow(result_fr, base_fr, exp_fr, MPFR_RNDN);

    mpfr_to_mpf(result, result_fr);

    mpfr_clear(base_fr);
    mpfr_clear(exp_fr);
    mpfr_clear(result_fr);
    cleanup_gmp_mpfr();
}

void mpf_pow_z(mpf_t result, mpz_t base, mpz_t exp)
{
    mpf_t base_pf, exp_pf;

    mpf_init(base_pf);
    mpf_init(exp_pf);

    mpf_set_z(base_pf, base);
    mpf_set_z(exp_pf, exp);

    mpf_pow(result, base_pf, exp_pf);

    mpf_clear(exp_pf);
    mpf_clear(base_pf);
}

int mpf_is_int(mpf_t num)
{
    mpz_t int_part;
    mpf_t temp;

    mpz_init(int_part);
    mpf_init(temp);

    mpz_set_f(int_part, num);
    mpf_set_z(temp, int_part);

    int result = (mpf_cmp(num, temp) == 0);

    mpz_clear(int_part);
    mpf_clear(temp);

    return result;
}

not_record_t *
not_power_pow(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->null)
    {
        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2;

            mpz_init_set(num1, (*(mpz_t *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NULL;

            if (mpf_is_int(result))
            {
                record = not_record_make_int_from_f(result);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t num1, result;

            mpf_init(result);
            mpf_init(num1);

            mpf_set_z(num1, (*(mpz_t *)(left->value)));

            mpf_pow(result, num1, (*(mpf_t *)(right->value)));
            mpf_clear(num1);

            not_record_t *record = NULL;
            if (mpf_is_int(result))
            {
                record = not_record_make_int_from_f(result);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num1, num2;

            mpz_init_set(num1, (*(mpz_t *)(left->value)));
            mpz_init_set_si(num2, (*(char *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NULL;

            if (mpf_is_int(result))
            {
                record = not_record_make_int_from_f(result);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2;
            mpz_init(num1);

            mpz_set_f(num1, (*(mpf_t *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t result;

            mpf_init(result);

            mpf_pow(result, (*(mpf_t *)(left->value)), (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpf_t num2, result;

            mpf_init(result);

            mpf_init_set_si(num2, (*(char *)(right->value)));

            mpf_pow(result, (*(mpf_t *)(left->value)), num2);
            mpf_clear(num2);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2;

            mpz_init_set_si(num1, (*(char *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NULL;

            if (mpf_is_int(result))
            {
                record = not_record_make_int_from_f(result);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t num1, result;

            mpf_init(result);

            mpf_init_set_si(num1, (*(char *)(left->value)));

            mpf_pow(result, num1, (*(mpf_t *)(right->value)));
            mpf_clear(num1);

            not_record_t *record = NULL;
            if (mpf_is_int(result))
            {
                record = not_record_make_int_from_f(result);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num1, num2;

            mpz_init_set_si(num1, (*(char *)(left->value)));
            mpz_init_set_si(num2, (*(char *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NULL;

            if (mpf_is_int(result))
            {
                record = not_record_make_int_from_f(result);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, "**", applicant);
    }

    return not_record_make_nan();
}

not_record_t *
not_power(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_POW)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_power(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_prefix(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_power_pow(node, left, right, applicant);

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
        return not_prefix(node, strip, applicant, origin);
    }
}

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

not_record_t *
not_shifting_shr(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
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
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, ">>", applicant);
    }

    return not_record_make_undefined();
}

not_record_t *
not_shifting_shl(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
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
            mpz_t num, shift_amount, result;

            mpz_init(result);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, "<<", applicant);
    }

    return not_record_make_undefined();
}

not_record_t *
not_shifting(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_SHR)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_addative(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_addative(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_shifting_shr(node, left, right, applicant);

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
    else if (node->kind == NODE_KIND_SHL)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_addative(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_addative(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_shifting_shl(node, left, right, applicant);

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
        return not_addative(node, strip, applicant, origin);
    }
}

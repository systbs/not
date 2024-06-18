#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <jansson.h>

#include "../../types/types.h"
#include "../../container/queue.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../../parser/syntax/syntax.h"
#include "../../error.h"
#include "../../mutex.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

not_record_t *
not_execute_eq(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(1);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(0);
        }
        else
        {
            return not_record_make_int_from_si(0);
        }
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(1);
        }
        else
        {
            return not_record_make_int_from_si(0);
        }
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp((*(mpz_t *)(left->value)), (*(mpz_t *)(right->value))) == 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(left->value)));
            int32_t result = (mpf_cmp(mpf_from_mpz, (*(mpf_t *)(right->value))) == 0);
            mpf_clear(mpf_from_mpz);
            return not_record_make_int_from_si(result);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), (*(char *)(right->value))) == 0);
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(right->value)));
            int32_t result = (mpf_cmp((*(mpf_t *)(left->value)), mpf_from_mpz) == 0);
            mpf_clear(mpf_from_mpz);
            return not_record_make_int_from_si(result);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp((*(mpf_t *)(left->value)), (*(mpf_t *)(right->value))) == 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(left->value)), (*(char *)(right->value))) == 0);
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_si((*(mpz_t *)(right->value)), (*(char *)(left->value))) == 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(right->value)), (*(char *)(left->value))) == 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si((*(char *)(left->value)) == (*(char *)(right->value)));
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_int_from_si(strcmp(((char *)(left->value)), ((char *)(right->value))) == 0);
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, "==", applicant);
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(0);
        }

        return not_record_make_int_from_si(0);
    }
    return not_record_make_int_from_si(0);
}

not_record_t *
not_execute_equality(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_EQ)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_execute_relational(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_execute_relational(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_execute_eq(node, left, right, applicant);

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
    else if (node->kind == NODE_KIND_NEQ)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_execute_relational(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_execute_relational(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_execute_eq(node, left, right, applicant);

        if (not_record_link_decrease(left) < 0)
        {
            return NOT_PTR_ERROR;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return NOT_PTR_ERROR;
        }

        if (record == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        left = not_record_make_int_from_si(1);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        right = record;

        record = not_execute_eq(node, left, right, applicant);

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
        return not_execute_relational(node, strip, applicant, origin);
    }
}

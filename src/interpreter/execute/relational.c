#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>

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

sy_record_t *
sy_execute_lt(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        {
            return sy_record_make_int_from_si(0);
        }
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        {
            return sy_record_make_int_from_si(0);
        }
    }
    else
    if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(mpz_cmp((*(mpz_t *)(left->value)), (*(mpz_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(left->value)));
            int32_t result = (mpf_cmp(mpf_from_mpz, (*(mpf_t *)(right->value))) < 0);
            mpf_clear(mpf_from_mpz);
            return sy_record_make_int_from_si(result);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), (*(char *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }
        
        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(right->value)));
            int32_t result = (mpf_cmp((*(mpf_t *)(left->value)), mpf_from_mpz) < 0);
            mpf_clear(mpf_from_mpz);
            return sy_record_make_int_from_si(result);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(mpf_cmp((*(mpf_t *)(left->value)), (*(mpf_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(left->value)), (*(char *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }
        
        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(mpz_cmp_si((*(mpz_t *)(right->value)), (*(char *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(right->value)), (*(char *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si((*(char *)(left->value)) < (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(strcmp(((char *)(left->value)), ((char *)(right->value))) >= 0);

        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        return sy_execute_call_for_operator_by_one_argument(node, left, right, "<", applicant);
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    return sy_record_make_int_from_si(0);
}

sy_record_t *
sy_execute_le(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(1);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        {
            return sy_record_make_int_from_si(0);
        }
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(1);
        }
        else
        {
            return sy_record_make_int_from_si(0);
        }
    }
    else
    if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(mpz_cmp((*(mpz_t *)(left->value)), (*(mpz_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(left->value)));
            int32_t result = (mpf_cmp(mpf_from_mpz, (*(mpf_t *)(right->value))) <= 0);
            mpf_clear(mpf_from_mpz);
            return sy_record_make_int_from_si(result);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), (*(char *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }
        
        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(right->value)));
            int32_t result = (mpf_cmp((*(mpf_t *)(left->value)), mpf_from_mpz) <= 0);
            mpf_clear(mpf_from_mpz);
            return sy_record_make_int_from_si(result);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(mpf_cmp((*(mpf_t *)(left->value)), (*(mpf_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(left->value)), (*(char *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }
        
        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(mpz_cmp_si((*(mpz_t *)(right->value)), (*(char *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(right->value)), (*(char *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si((*(char *)(left->value)) <= (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(strcmp(((char *)(left->value)), ((char *)(right->value))) <= 0);

        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        return sy_execute_call_for_operator_by_one_argument(node, left, right, "<=", applicant);
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    
    return sy_record_make_int_from_si(0);
}

sy_record_t *
sy_execute_gt(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        {
            return sy_record_make_int_from_si(0);
        }
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        {
            return sy_record_make_int_from_si(0);
        }
    }
    else
    if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(mpz_cmp((*(mpz_t *)(left->value)), (*(mpz_t *)(right->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(left->value)));
            int32_t result = (mpf_cmp(mpf_from_mpz, (*(mpf_t *)(right->value))) > 0);
            mpf_clear(mpf_from_mpz);
            return sy_record_make_int_from_si(result);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), (*(char *)(right->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }
        
        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(right->value)));
            int32_t result = (mpf_cmp((*(mpf_t *)(left->value)), mpf_from_mpz) < 0);
            mpf_clear(mpf_from_mpz);
            return sy_record_make_int_from_si(result);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(mpf_cmp((*(mpf_t *)(left->value)), (*(mpf_t *)(right->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(left->value)), (*(char *)(right->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }
        
        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(mpz_cmp_si((*(mpz_t *)(right->value)), (*(char *)(left->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(right->value)), (*(char *)(left->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si((*(char *)(left->value)) > (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(strcmp(((char *)(left->value)), ((char *)(right->value))) <= 0);

        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        return sy_execute_call_for_operator_by_one_argument(node, left, right, ">", applicant);
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    return sy_record_make_int_from_si(0);
}

sy_record_t *
sy_execute_ge(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(1);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        {
            return sy_record_make_int_from_si(0);
        }
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(1);
        }
        else
        {
            return sy_record_make_int_from_si(0);
        }
    }
    else
    if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(mpz_cmp((*(mpz_t *)(left->value)), (*(mpz_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(left->value)));
            int32_t result = (mpf_cmp(mpf_from_mpz, (*(mpf_t *)(right->value))) <= 0);
            mpf_clear(mpf_from_mpz);
            return sy_record_make_int_from_si(result);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), (*(char *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }
        
        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(right->value)));
            int32_t result = (mpf_cmp((*(mpf_t *)(left->value)), mpf_from_mpz) <= 0);
            mpf_clear(mpf_from_mpz);
            return sy_record_make_int_from_si(result);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(mpf_cmp((*(mpf_t *)(left->value)), (*(mpf_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(left->value)), (*(char *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }
        
        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(mpz_cmp_si((*(mpz_t *)(right->value)), (*(char *)(left->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(right->value)), (*(char *)(left->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si((*(char *)(left->value)) >= (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(strcmp(((char *)(left->value)), ((char *)(right->value))) <= 0);

        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        return sy_execute_call_for_operator_by_one_argument(node, left, right, ">=", applicant);
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_int_from_si(0);
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_int_from_si(0);
        }

        return sy_record_make_int_from_si(0);
    }
    
    return sy_record_make_int_from_si(0);
}


sy_record_t *
sy_execute_relational(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_LT)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_shifting(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_shifting(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }
        
        sy_record_t *record = sy_execute_lt(node, left, right, applicant);
        
        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_LE)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_shifting(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_shifting(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_le(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_GT)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_shifting(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_shifting(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_gt(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_GE)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_shifting(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_shifting(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_ge(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    {
        return sy_execute_shifting(node, strip, applicant, origin);
    }
}

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
not_equality_eq(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->null || left->undefined || left->nan)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(1);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(1);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(right->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(right->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(*(char *)(right->value) == 0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), 0) == 0);
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
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), 0) == 0);
        }

        return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(left->value)), 0) != 0);
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(left->value)), 0) == 0);
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
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(left->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(left->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(left->value)), 0) == 0);
        }

        return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(left->value)), 0) != 0);
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si((*(char *)(left->value)) == 0);
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
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si((*(char *)(left->value)) == 0);
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si((*(char *)(left->value)) == 0);
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si((*(char *)(left->value)) == 0);
        }

        return not_record_make_int_from_si((*(char *)(left->value)) != 0);
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_int_from_si(strcmp(((char *)(left->value)), ((char *)(right->value))) == 0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(*(char *)(right->value) != 0);
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            size_t left_count = 0, right_count = 0;
            not_record_object_t *object_left;
            not_record_object_t *object_right;
            for (object_left = (not_record_object_t *)left->value; object_left != NULL; object_left = object_left->next)
            {
                left_count += 1;
                int found = 0;
                for (object_right = (not_record_object_t *)right->value; object_right != NULL; object_right = object_right->next)
                {
                    if (strcmp(object_left->key, object_right->key) == 0)
                    {
                        found = 1;

                        not_record_t *r = not_equality_eq(node, object_left->value, object_right->value, applicant);
                        if (r == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }

                        if (!not_execute_truthy(r))
                        {
                            if (not_record_link_decrease(r) < 0)
                            {
                                return NOT_PTR_ERROR;
                            }
                            return not_record_make_int_from_si(0);
                        }

                        if (not_record_link_decrease(r) < 0)
                        {
                            return NOT_PTR_ERROR;
                        }

                        break;
                    }
                }

                if (!found)
                {
                    return not_record_make_int_from_si(0);
                }
            }

            for (object_right = (not_record_object_t *)right->value; object_right != NULL; object_right = object_right->next)
            {
                right_count += 1;
            }

            if (left_count != right_count)
            {
                return not_record_make_int_from_si(0);
            }

            return not_record_make_int_from_si(1);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(*(char *)(right->value) != 0);
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            size_t left_count = 0, right_count = 0;

            not_record_tuple_t *tuple_left;
            not_record_tuple_t *tuple_right;
            for (tuple_left = (not_record_tuple_t *)left->value, tuple_right = (not_record_tuple_t *)right->value; tuple_left != NULL; tuple_left = tuple_left->next, tuple_right = tuple_right->next)
            {
                left_count += 1;

                if (tuple_right == NULL)
                {
                    return not_record_make_int_from_si(0);
                }

                not_record_t *r = not_equality_eq(node, tuple_left->value, tuple_right->value, applicant);
                if (r == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }

                if (!not_execute_truthy(r))
                {
                    if (not_record_link_decrease(r) < 0)
                    {
                        return NOT_PTR_ERROR;
                    }
                    return not_record_make_int_from_si(0);
                }

                if (not_record_link_decrease(r) < 0)
                {
                    return NOT_PTR_ERROR;
                }
            }

            for (tuple_right = (not_record_tuple_t *)right->value; tuple_right != NULL; tuple_right = tuple_right->next)
            {
                right_count += 1;
            }

            if (left_count != right_count)
            {
                return not_record_make_int_from_si(0);
            }

            return not_record_make_int_from_si(1);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, "==", applicant);
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(*(char *)(right->value) != 0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(1);
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_int_from_si(1);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_si((*(mpz_t *)(right->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(right->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si((*(char *)(right->value)) == 0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_int_from_si(1);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_si((*(mpz_t *)(right->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(right->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si((*(char *)(right->value)) == 0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_int_from_si(1);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_si((*(mpz_t *)(right->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_si((*(mpf_t *)(right->value)), 0) == 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si((*(char *)(right->value)) == 0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_PROC)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(*(char *)(right->value) != 0);
        }

        return not_record_make_int_from_si(0);
    }
    else if (left->kind == RECORD_KIND_BUILTIN)
    {
        if (right->null || right->undefined || right->nan)
        {
            return not_record_make_int_from_si(0);
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            return not_record_make_int_from_si(mpz_cmp_d((*(mpz_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            return not_record_make_int_from_si(mpf_cmp_d((*(mpf_t *)(right->value)), 0) != 0);
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            return not_record_make_int_from_si(*(char *)(right->value) != 0);
        }

        return not_record_make_int_from_si(0);
    }

    return not_record_make_int_from_si(0);
}

not_record_t *
not_equality(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_EQ)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_relational(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_relational(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_equality_eq(node, left, right, applicant);

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
        not_record_t *left = not_relational(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_relational(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_equality_eq(node, left, right, applicant);

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

        record = not_equality_eq(node, left, right, applicant);

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
        return not_relational(node, strip, applicant, origin);
    }
}

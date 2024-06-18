#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <float.h>
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
#include "../entry.h"
#include "execute.h"

static int32_t
size_of(not_record_t *record, size_t *size)
{
    if (record->kind == RECORD_KIND_INT)
    {
        size_t num_limbs = mpz_size(*(mpz_t *)record->value);
        size_t limb_size = sizeof(mp_limb_t);
        *size += num_limbs * limb_size;
        return 0;
    }
    else if (record->kind == RECORD_KIND_FLOAT)
    {
        size_t num_limbs = mpf_size(*(mpf_t *)record->value);
        size_t limb_size = sizeof(mp_limb_t);
        *size += num_limbs * limb_size;
        return 0;
    }
    else if (record->kind == RECORD_KIND_CHAR)
    {
        *size += 0;
        return 0;
    }
    else if (record->kind == RECORD_KIND_STRING)
    {
        *size += strlen((char *)record->value);
        return 0;
    }
    else if (record->kind == RECORD_KIND_OBJECT)
    {
        for (not_record_object_t *item = (not_record_object_t *)record->value; item != NOT_PTR_NULL; item = item->next)
        {
            int32_t r = size_of(item->value, size);
            if (r < 0)
            {
                return -1;
            }
        }
        return 0;
    }
    else if (record->kind == RECORD_KIND_TUPLE)
    {
        for (not_record_object_t *item = (not_record_object_t *)record->value; item != NOT_PTR_NULL; item = item->next)
        {
            int32_t r = size_of(item->value, size);
            if (r < 0)
            {
                return -1;
            }
        }
        return 0;
    }
    else if (record->kind == RECORD_KIND_STRUCT)
    {
        not_record_struct_t *struct1 = (not_record_struct_t *)record->value;
        not_node_t *type = struct1->type;
        not_strip_t *strip_class = struct1->value;

        not_node_class_t *class1 = (not_node_class_t *)type->value;

        for (not_node_t *item = class1->block; item != NOT_PTR_NULL; item = item->next)
        {
            if (item->kind == NODE_KIND_PROPERTY)
            {
                not_node_property_t *property = (not_node_property_t *)item->value;
                if ((property->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                {
                    continue;
                }

                if ((property->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    continue;
                }

                not_entry_t *entry = not_strip_variable_find(strip_class, type, property->key);
                if (entry == NOT_PTR_ERROR)
                {
                    return -1;
                }
                if (entry == NOT_PTR_NULL)
                {
                    not_node_basic_t *basic = (not_node_basic_t *)property->key->value;
                    not_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    return -1;
                }

                int32_t r = size_of(entry->value, size);
                if (r < 0)
                {
                    return -1;
                }

                if (not_record_link_decrease(entry->value) < 0)
                {
                    return -1;
                }
            }
        }

        if (class1->heritages)
        {
            not_node_block_t *block = (not_node_block_t *)class1->heritages->value;
            for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
            {
                not_node_heritage_t *heritage = (not_node_heritage_t *)item->value;
                not_entry_t *entry = not_strip_variable_find(strip_class, type, heritage->key);
                if (entry == NOT_PTR_ERROR)
                {
                    return -1;
                }
                if (entry == NOT_PTR_NULL)
                {
                    not_node_basic_t *basic = (not_node_basic_t *)heritage->key->value;
                    not_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    return -1;
                }

                int32_t r = size_of(entry->value, size);
                if (r < 0)
                {
                    return -1;
                }

                if (not_record_link_decrease(entry->value) < 0)
                {
                    return -1;
                }
            }
        }

        return 0;
    }

    return 0;
}

not_record_t *
not_execute_prefix(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_TILDE)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;

        not_record_t *left = not_record_make_int_from_si(-1);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_execute_prefix(unary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_execute_xor(node, left, right, applicant);

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
    else if (node->kind == NODE_KIND_POS)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;

        not_record_t *left = not_record_make_int_from_si(0);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_execute_prefix(unary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_execute_plus(node, left, right, applicant);

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
    else if (node->kind == NODE_KIND_NEG)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;

        not_record_t *left = not_record_make_int_from_si(0);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_execute_prefix(unary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_execute_minus(node, left, right, applicant);

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
    else if (node->kind == NODE_KIND_NOT)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;

        not_record_t *left = not_record_make_int_from_si(0);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_execute_prefix(unary->right, strip, applicant, origin);
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
    else if (node->kind == NODE_KIND_SIZEOF)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;

        not_record_t *right = not_execute_prefix(unary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        size_t size = 0;
        int32_t r = size_of(right, &size);
        if (r < 0)
        {
            if (not_record_link_decrease(right) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_record_make_int_from_ui(size);

        if (not_record_link_decrease(right) < 0)
        {
            return NOT_PTR_ERROR;
        }

        return record;
    }
    else if (node->kind == NODE_KIND_TYPEOF)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;

        not_record_t *right = not_execute_prefix(unary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        char *type_string = "null";
        if (right->kind == RECORD_KIND_INT)
        {
            type_string = "int";
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            type_string = "float";
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            type_string = "char";
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            type_string = "string";
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            type_string = "object";
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            type_string = "tuple";
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            type_string = "struct";
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            type_string = "type";
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            type_string = "undefined";
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            type_string = "nan";
        }

        not_record_t *record = not_record_make_string(type_string);

        if (not_record_link_decrease(right) < 0)
        {
            return NOT_PTR_ERROR;
        }

        return record;
    }
    else
    {
        return not_execute_postfix(node, strip, applicant, origin);
    }
}

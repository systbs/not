#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <float.h>

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
size_of(sy_record_t *record, size_t *size)
{
    if (record->kind == RECORD_KIND_INT)
    {
        size_t num_limbs = mpz_size(*(mpz_t *)record->value);
        size_t limb_size = sizeof(mp_limb_t);
        *size += num_limbs * limb_size;
        return 0;
    }
    else
    if (record->kind == RECORD_KIND_FLOAT)
    {
        size_t num_limbs = mpf_size(*(mpf_t *)record->value);
        size_t limb_size = sizeof(mp_limb_t);
        *size += num_limbs * limb_size;
        return 0;
    }
    else
    if (record->kind == RECORD_KIND_CHAR)
    {
        *size += 0;
        return 0;
    }
    else
    if (record->kind == RECORD_KIND_STRING)
    {
        *size += strlen((char *)record->value);
        return 0;
    }
    else
    if (record->kind == RECORD_KIND_OBJECT)
    {
        for (sy_record_object_t *item = (sy_record_object_t *)record->value;item != NULL;item = item->next)
        {
            int32_t r = size_of(item->value, size);
            if (r < 0)
            {
                return -1;
            }
        }
        return 0;
    }
    else
    if (record->kind == RECORD_KIND_TUPLE)
    {
        for (sy_record_object_t *item = (sy_record_object_t *)record->value;item != NULL;item = item->next)
        {
            int32_t r = size_of(item->value, size);
            if (r < 0)
            {
                return -1;
            }
        }
        return 0;
    }
    else
    if (record->kind == RECORD_KIND_STRUCT)
    {
        sy_record_struct_t *struct1 = (sy_record_struct_t *)record->value;
        sy_node_t *type = struct1->type;
        sy_strip_t *strip_class = struct1->value;

        sy_node_class_t *class1 = (sy_node_class_t *)type->value;

        for (sy_node_t *item = class1->block;item != NULL;item = item->next)
        {
            if (item->kind == NODE_KIND_PROPERTY)
            {
                sy_node_property_t *property = (sy_node_property_t *)item->value;
                if ((property->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                {
                    continue;
                }

                if ((property->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    continue;
                }

                sy_entry_t *entry = sy_strip_variable_find(strip_class, type, property->key);
                if (entry == ERROR)
                {
                    return -1;
                }
                if (entry == NULL)
                {
                    sy_node_basic_t *basic = (sy_node_basic_t *)property->key->value;
                    sy_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    return -1;
                }

                int32_t r = size_of(entry->value, size);
                if (r < 0)
                {
                    return -1;
                }
                
                entry->value->link -= 1;
            }
        }

        if (class1->heritages)
        {
            sy_node_block_t *block = (sy_node_block_t *)class1->heritages->value;
            for (sy_node_t *item = block->items; item != NULL; item = item->next)
            {
                sy_node_heritage_t *heritage = (sy_node_heritage_t *)item->value;
                sy_entry_t *entry = sy_strip_variable_find(strip_class, type, heritage->key);
                if (entry == ERROR)
                {
                    return -1;
                }
                if (entry == NULL)
                {
                    sy_node_basic_t *basic = (sy_node_basic_t *)heritage->key->value;
                    sy_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    return -1;
                }

                int32_t r = size_of(entry->value, size);
                if (r < 0)
                {
                    return -1;
                }
                
                entry->value->link -= 1;
            }
        }

        return 0;

    }

    return 0;
}

sy_record_t *
sy_execute_prefix(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_TILDE)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

        sy_record_t *left = sy_record_make_int_from_si(-1);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_prefix(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_xor(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_POS)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

        sy_record_t *left = sy_record_make_int_from_si(0);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_prefix(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_plus(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_NEG)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

        sy_record_t *left = sy_record_make_int_from_si(0);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_prefix(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_minus(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_NOT)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

        sy_record_t *left = sy_record_make_int_from_si(0);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_prefix(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_eq(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_SIZEOF)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

        sy_record_t *right = sy_execute_prefix(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        size_t size = 0;
        int32_t r = size_of(right, &size);
        if (r < 0)
        {
            right->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_record_make_int_from_ui(size);

        right->link -= 1;

        return record;
    }
    else
    {
        return sy_execute_postfix(node, strip, applicant, origin);
    }
}


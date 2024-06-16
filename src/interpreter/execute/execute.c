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
#include "../../memory.h"
#include "../../interpreter.h"
#include "../../thread.h"
#include "../../config.h"
#include "../record.h"
#include "../garbage.h"
#include "../entry.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

int32_t
not_execute_id_cmp(not_node_t *n1, not_node_t *n2)
{
    not_node_basic_t *nb1 = (not_node_basic_t *)n1->value;
    not_node_basic_t *nb2 = (not_node_basic_t *)n2->value;

    return (strcmp(nb1->value, nb2->value) == 0);
}

int32_t
not_execute_id_strcmp(not_node_t *n1, const char *name)
{
    not_node_basic_t *nb1 = (not_node_basic_t *)n1->value;

    return (strcmp(nb1->value, name) == 0);
}

static int32_t
not_execute_body(not_node_t *node, not_strip_t *strip, not_node_t *applicant);

int32_t
not_execute_truthy(not_record_t *left)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        return 0;
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        return 0;
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        return !(mpz_cmp_si((*(mpz_t *)(left->value)), 0) == 0);
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        return !(mpf_cmp_d((*(mpf_t *)(left->value)), 0.0) == 0);
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        return !((*(char *)(left->value)) == 0);
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        return 1;
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        return 1;
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        return 1;
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        return 1;
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return 1;
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        return 0;
    }
    return 0;
}

int32_t
not_execute_type_check_by_type(not_node_t *node, not_record_t *record_type1, not_record_t *record_type2, not_strip_t *strip, not_node_t *applicant)
{
    not_record_type_t *type1 = (not_record_type_t *)record_type1->value;
    not_record_type_t *type2 = (not_record_type_t *)record_type2->value;

    if (type1->type->kind == NODE_KIND_KINT)
    {
        if (type2->type->kind == NODE_KIND_KINT)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (type1->type->kind == NODE_KIND_KFLOAT)
    {
        if (type2->type->kind == NODE_KIND_KFLOAT)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (type1->type->kind == NODE_KIND_OBJECT)
    {
        if (type2->type->kind == RECORD_KIND_OBJECT)
        {
            uint64_t cnt1 = 0;
            for (not_record_object_t *object1 = (not_record_object_t *)type1->value; object1 != NULL; object1 = object1->next)
            {
                int32_t found = 0;
                cnt1 += 1;
                for (not_record_object_t *object2 = (not_record_object_t *)type2->value; object2 != NULL; object2 = object2->next)
                {
                    if (strcmp(object1->key, object2->key) == 0)
                    {
                        int32_t r1 = not_execute_type_check_by_type(node, object1->value, object2->value, strip, applicant);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else if (r1 == 0)
                        {
                            return 0;
                        }
                        found = 1;
                    }
                }
                if (found == 0)
                {
                    return 0;
                }
            }

            uint64_t cnt2 = 0;
            for (not_record_object_t *object2 = (not_record_object_t *)type2->value; object2 != NULL; object2 = object2->next)
            {
                cnt2 += 1;
            }

            if (cnt1 != cnt2)
            {
                return 0;
            }

            return 1;
        }
        return 0;
    }
    else if (type1->type->kind == NODE_KIND_ARRAY)
    {
        if (type2->type->kind == NODE_KIND_ARRAY)
        {
            int32_t r1 = not_execute_type_check_by_type(node, type1->value, type2->value, strip, applicant);
            if (r1 == -1)
            {
                return -1;
            }
            else if (r1 == 0)
            {
                return 0;
            }
            return 1;
        }
        return 0;
    }
    else if (type1->type->kind == NODE_KIND_TUPLE)
    {
        if (type2->type->kind == NODE_KIND_TUPLE)
        {
            uint64_t cnt1 = 0;
            for (
                not_record_tuple_t *tuple1 = (not_record_tuple_t *)type1->value, *tuple2 = (not_record_tuple_t *)type2->value;
                tuple1 != NULL;
                tuple1 = tuple1->next, tuple2 = tuple2->next)
            {
                cnt1 += 1;

                if (tuple2 == NULL)
                {
                    return 0;
                }

                int32_t r1 = not_execute_type_check_by_type(node, tuple2->value, tuple1->value, strip, applicant);
                if (r1 == -1)
                {
                    return -1;
                }
                else if (r1 == 0)
                {
                    return 0;
                }
            }

            uint64_t cnt2 = 0;
            for (not_record_tuple_t *tuple2 = (not_record_tuple_t *)type2->value; tuple2 != NULL; tuple2 = tuple2->next)
            {
                cnt2 += 1;
            }

            if (cnt1 != cnt2)
            {
                return 0;
            }

            return 1;
        }

        return 0;
    }
    else if (type1->type->kind == NODE_KIND_CLASS)
    {
        if (type2->type->kind == NODE_KIND_CLASS)
        {
            if (type1->type->id != type2->type->id)
            {
                return 0;
            }

            not_node_class_t *class1 = (not_node_class_t *)type1->type->value;
            if (class1->generics)
            {
                not_node_block_t *block1 = (not_node_block_t *)class1->generics->value;
                for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                {
                    not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;

                    not_entry_t *strip_entry1 = not_strip_variable_find(type1->value, type1->type, generic1->key);
                    if (strip_entry1 == ERROR)
                    {
                        return -1;
                    }
                    if (strip_entry1 == NULL)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;
                        not_error_runtime_by_node(node, "'%s' is not initialized", basic1->value);
                        return -1;
                    }

                    not_entry_t *strip_entry2 = not_strip_variable_find(type2->value, type2->type, generic1->key);
                    if (strip_entry2 == ERROR)
                    {
                        if (not_record_link_decrease(strip_entry1->value) < 0)
                        {
                            return -1;
                        }
                        return -1;
                    }
                    if (strip_entry2 == NULL)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;
                        not_error_runtime_by_node(node, "'%s' is not initialized", basic1->value);
                        if (not_record_link_decrease(strip_entry1->value) < 0)
                        {
                            return -1;
                        }
                        return -1;
                    }

                    int32_t r1 = not_execute_type_check_by_type(node, strip_entry1->value, strip_entry2->value, strip, applicant);
                    if (r1 == -1)
                    {
                        if (not_record_link_decrease(strip_entry1->value) < 0)
                        {
                            return -1;
                        }
                        if (not_record_link_decrease(strip_entry2->value) < 0)
                        {
                            return -1;
                        }
                        return -1;
                    }
                    else if (r1 == 0)
                    {
                        if (not_record_link_decrease(strip_entry1->value) < 0)
                        {
                            return -1;
                        }
                        if (not_record_link_decrease(strip_entry2->value) < 0)
                        {
                            return -1;
                        }
                        return 0;
                    }
                }
            }

            return 1;
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

int32_t
not_execute_value_check_by_type(not_node_t *node, not_record_t *record_value, not_record_t *record_type, not_strip_t *strip, not_node_t *applicant)
{
    not_record_type_t *type1 = (not_record_type_t *)record_type->value;

    if (type1->type->kind == NODE_KIND_KINT)
    {
        if (record_value->kind == RECORD_KIND_INT)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (type1->type->kind == NODE_KIND_KFLOAT)
    {
        if (record_value->kind == RECORD_KIND_FLOAT)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (type1->type->kind == NODE_KIND_OBJECT)
    {
        if (record_value->kind == RECORD_KIND_OBJECT)
        {
            uint64_t cnt1 = 0;
            for (not_record_object_t *object1 = (not_record_object_t *)record_value->value; object1 != NULL; object1 = object1->next)
            {
                int32_t found = 0;
                cnt1 += 1;
                for (not_record_object_t *object2 = (not_record_object_t *)type1->value; object2 != NULL; object2 = object2->next)
                {
                    if (strcmp(object1->key, object2->key) == 0)
                    {
                        int32_t r1 = not_execute_value_check_by_type(node, object1->value, object2->value, strip, applicant);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else if (r1 == 0)
                        {
                            return 0;
                        }
                        found = 1;
                    }
                }
                if (found == 0)
                {
                    return 0;
                }
            }

            uint64_t cnt2 = 0;
            for (not_record_object_t *object2 = (not_record_object_t *)type1->value; object2 != NULL; object2 = object2->next)
            {
                cnt2 += 1;
            }

            if (cnt1 != cnt2)
            {
                return 0;
            }

            return 1;
        }
        return 0;
    }
    else if (type1->type->kind == NODE_KIND_ARRAY)
    {
        if (record_value->kind == RECORD_KIND_TUPLE)
        {
            for (not_record_tuple_t *tuple = (not_record_tuple_t *)record_value->value; tuple != NULL; tuple = tuple->next)
            {
                int32_t r1 = not_execute_value_check_by_type(node, tuple->value, (not_record_t *)type1->value, strip, applicant);
                if (r1 == -1)
                {
                    return -1;
                }
                else if (r1 == 0)
                {
                    return 0;
                }
            }

            return 1;
        }

        return 0;
    }
    else if (type1->type->kind == NODE_KIND_TUPLE)
    {
        if (record_value->kind == RECORD_KIND_TUPLE)
        {
            uint64_t cnt1 = 0;
            for (
                not_record_tuple_t *tuple1 = (not_record_tuple_t *)type1->value, *tuple2 = (not_record_tuple_t *)record_value->value;
                tuple1 != NULL;
                tuple1 = tuple1->next, tuple2 = tuple2->next)
            {
                cnt1 += 1;

                if (tuple2 == NULL)
                {
                    return 0;
                }

                int32_t r1 = not_execute_value_check_by_type(node, tuple2->value, tuple1->value, strip, applicant);
                if (r1 == -1)
                {
                    return -1;
                }
                else if (r1 == 0)
                {
                    return 0;
                }
            }

            uint64_t cnt2 = 0;
            for (not_record_tuple_t *tuple2 = (not_record_tuple_t *)record_value->value; tuple2 != NULL; tuple2 = tuple2->next)
            {
                cnt2 += 1;
            }

            if (cnt1 != cnt2)
            {
                return 0;
            }

            return 1;
        }

        return 0;
    }
    else if (type1->type->kind == NODE_KIND_CLASS)
    {
        if (record_value->kind == RECORD_KIND_STRUCT)
        {
            not_record_struct_t *struct1 = (not_record_struct_t *)record_value->value;
            if (type1->type->id != struct1->type->id)
            {
                return 0;
            }

            not_node_class_t *class1 = (not_node_class_t *)struct1->type->value;
            if (class1->generics)
            {
                not_node_block_t *block1 = (not_node_block_t *)class1->generics->value;
                if (block1 != NULL)
                {
                    for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                    {
                        not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;

                        not_entry_t *strip_entry1 = not_strip_variable_find(struct1->value, struct1->type, generic1->key);
                        if (strip_entry1 == ERROR)
                        {
                            return -1;
                        }
                        if (strip_entry1 == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;
                            not_error_runtime_by_node(node, "'%s' is not initialized", basic1->value);
                            return -1;
                        }

                        not_entry_t *strip_entry2 = not_strip_variable_find(type1->value, type1->type, generic1->key);
                        if (strip_entry2 == ERROR)
                        {
                            if (not_record_link_decrease(strip_entry1->value) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }
                        if (strip_entry2 == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;
                            not_error_runtime_by_node(node, "'%s' is not initialized", basic1->value);
                            if (not_record_link_decrease(strip_entry1->value) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }

                        int32_t r1 = not_execute_type_check_by_type(node, strip_entry1->value, strip_entry2->value, strip, applicant);
                        if (r1 == -1)
                        {
                            if (not_record_link_decrease(strip_entry1->value) < 0)
                            {
                                return -1;
                            }
                            if (not_record_link_decrease(strip_entry2->value) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }
                        else if (r1 == 0)
                        {
                            if (not_record_link_decrease(strip_entry1->value) < 0)
                            {
                                return -1;
                            }
                            if (not_record_link_decrease(strip_entry2->value) < 0)
                            {
                                return -1;
                            }
                            return 0;
                        }
                    }
                }
            }
            return 1;
        }
        return 0;
    }
    return 0;
}

not_record_t *
not_execute_value_casting_by_type(not_node_t *node, not_record_t *record_value, not_record_t *record_type, not_strip_t *strip, not_node_t *applicant)
{
    not_record_type_t *type1 = (not_record_type_t *)record_type->value;

    if (type1->type->kind == NODE_KIND_KINT)
    {
        if (record_value->kind == RECORD_KIND_INT)
        {
            return record_value;
        }
        else
        {
            return NULL;
        }
    }
    else if (type1->type->kind == NODE_KIND_KFLOAT)
    {
        if (record_value->kind == RECORD_KIND_FLOAT)
        {
            return record_value;
        }
        else if (record_value->kind == RECORD_KIND_INT)
        {
            mpz_t val;
            mpz_init_set(val, *(mpz_t *)(record_value->value));
            void *ptr = not_memory_realloc(record_value->value, sizeof(mpf_t));
            if (!ptr)
            {
                not_error_no_memory();
                return ERROR;
            }
            mpf_init(*(mpf_t *)ptr);
            mpf_set_z(*(mpf_t *)ptr, val);
            mpz_clear(val);
            record_value->value = ptr;
            record_value->kind = RECORD_KIND_FLOAT;
            return record_value;
        }
        else
        {
            return NULL;
        }
    }
    else if (type1->type->kind == NODE_KIND_OBJECT)
    {
        if (record_value->kind == RECORD_KIND_OBJECT)
        {
            not_record_object_t *object_copy = not_record_object_copy((not_record_object_t *)record_value->value);
            if (object_copy == ERROR)
            {
                return ERROR;
            }

            uint64_t cnt1 = 0;
            for (not_record_object_t *object1 = (not_record_object_t *)type1->value; object1 != NULL; object1 = object1->next)
            {
                cnt1 += 1;
                int32_t found = 0;
                for (not_record_object_t *object2 = object_copy; object2 != NULL; object2 = object2->next)
                {
                    if (strcmp(object1->key, object2->key) == 0)
                    {
                        not_record_t *r1 = not_execute_value_casting_by_type(node, object2->value, object1->value, strip, applicant);
                        if (r1 == ERROR)
                        {
                            if (not_record_object_destroy(object_copy) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }
                        else if ((r1 == NULL) && (r1 != object2->value))
                        {
                            if (not_record_object_destroy(object_copy) < 0)
                            {
                                return ERROR;
                            }
                            return NULL;
                        }
                        object2->value = r1;
                        found = 1;
                        break;
                    }
                }
                if (found == 0)
                {
                    if (not_record_object_destroy(object_copy) < 0)
                    {
                        return ERROR;
                    }
                    return NULL;
                }
            }

            uint64_t cnt2 = 0;
            for (not_record_object_t *object2 = object_copy; object2 != NULL; object2 = object2->next)
            {
                cnt2 += 1;
            }

            if (cnt1 != cnt2)
            {
                if (not_record_object_destroy(object_copy) < 0)
                {
                    return ERROR;
                }
                return NULL;
            }

            if (not_record_object_destroy(record_value->value) < 0)
            {
                if (not_record_object_destroy(object_copy) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }

            record_value->value = object_copy;

            return record_value;
        }

        return NULL;
    }
    else if (type1->type->kind == NODE_KIND_ARRAY)
    {
        if (record_value->kind == RECORD_KIND_TUPLE)
        {
            not_record_tuple_t *tuple_copy = not_record_tuple_copy((not_record_tuple_t *)record_value->value);
            not_record_t *array_type = (not_record_t *)type1->value;

            for (not_record_tuple_t *tuple = tuple_copy; tuple != NULL; tuple = tuple->next)
            {
                not_record_t *r1 = not_execute_value_casting_by_type(node, tuple->value, array_type, strip, applicant);
                if (r1 == ERROR)
                {
                    if (not_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }
                else if ((r1 == NULL) && (r1 != tuple->value))
                {
                    if (not_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return NULL;
                }

                tuple->value = r1;
            }

            if (not_record_tuple_destroy(record_value->value) < 0)
            {
                return ERROR;
            }

            record_value->value = tuple_copy;

            return record_value;
        }

        return NULL;
    }
    else if (type1->type->kind == NODE_KIND_TUPLE)
    {
        if (record_value->kind == RECORD_KIND_TUPLE)
        {
            not_record_tuple_t *tuple_copy = not_record_tuple_copy((not_record_tuple_t *)record_value->value);

            uint64_t cnt1 = 0;
            for (not_record_tuple_t *tuple1 = (not_record_tuple_t *)type1->value, *tuple2 = tuple_copy; tuple1 != NULL; tuple1 = tuple1->next, tuple2 = tuple2->next)
            {
                cnt1 += 1;

                if (tuple2 == NULL)
                {
                    if (not_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return NULL;
                }

                not_record_t *r1 = not_execute_value_casting_by_type(node, tuple2->value, tuple1->value, strip, applicant);
                if (r1 == ERROR)
                {
                    if (not_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }
                else if ((r1 == NULL) && (r1 != tuple2->value))
                {
                    if (not_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return NULL;
                }

                tuple2->value = r1;
            }

            uint64_t cnt2 = 0;
            for (not_record_tuple_t *tuple2 = tuple_copy; tuple2 != NULL; tuple2 = tuple2->next)
            {
                cnt2 += 1;
            }

            if (cnt1 != cnt2)
            {
                if (not_record_tuple_destroy(tuple_copy) < 0)
                {
                    return ERROR;
                }
                return NULL;
            }

            if (not_record_tuple_destroy(record_value->value) < 0)
            {
                return ERROR;
            }

            record_value->value = tuple_copy;

            return record_value;
        }

        return NULL;
    }
    else if (type1->type->kind == NODE_KIND_CLASS)
    {
        if (record_value->kind == RECORD_KIND_STRUCT)
        {
            not_record_struct_t *struct1 = (not_record_struct_t *)record_value->value;
            if (type1->type->id != struct1->type->id)
            {
                return NULL;
            }

            not_node_class_t *class1 = (not_node_class_t *)struct1->type->value;
            if (class1->generics)
            {
                not_node_block_t *block1 = (not_node_block_t *)class1->generics->value;
                if (block1 != NULL)
                {
                    for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                    {
                        not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;

                        not_entry_t *strip_entry1 = not_strip_variable_find(struct1->value, struct1->type, generic1->key);
                        if (strip_entry1 == ERROR)
                        {
                            return ERROR;
                        }
                        if (strip_entry1 == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;
                            not_error_runtime_by_node(node, "'%s' is not initialized", basic1->value);
                            if (not_record_link_decrease(strip_entry1->value) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }

                        not_entry_t *strip_entry2 = not_strip_variable_find(type1->value, type1->type, generic1->key);
                        if (strip_entry2 == ERROR)
                        {
                            if (not_record_link_decrease(strip_entry1->value) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }
                        if (strip_entry2 == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;
                            not_error_runtime_by_node(node, "'%s' is not initialized", basic1->value);
                            if (not_record_link_decrease(strip_entry1->value) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }

                        int32_t r1 = not_execute_type_check_by_type(node, strip_entry1->value, strip_entry2->value, strip, applicant);
                        if (r1 == -1)
                        {
                            if (not_record_link_decrease(strip_entry2->value) < 0)
                            {
                                return ERROR;
                            }
                            if (not_record_link_decrease(strip_entry1->value) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }
                        else if (r1 == 0)
                        {
                            if (not_record_link_decrease(strip_entry2->value) < 0)
                            {
                                return ERROR;
                            }
                            if (not_record_link_decrease(strip_entry1->value) < 0)
                            {
                                return ERROR;
                            }
                            return NULL;
                        }
                    }
                }
            }
            return record_value;
        }
        return NULL;
    }
    return NULL;
}

static int32_t
not_execute_entity(not_node_t *scope, not_node_t *node, not_record_t *value, not_strip_t *strip, not_node_t *applicant)
{
    not_node_entity_t *entity = (not_node_entity_t *)node->value;
    not_node_t *key_search = entity->key;
    if (entity->type)
    {
        key_search = entity->type;
    }

    not_record_t *value_select = NULL;

    if (value)
    {
        if (value->kind == RECORD_KIND_STRUCT)
        {
            not_record_struct_t *record_struct = (not_record_struct_t *)value->value;
            not_node_t *type = record_struct->type;
            not_strip_t *strip_new = (not_strip_t *)record_struct->value;
            value_select = not_execute_attribute_from_struct(node, strip_new, type, key_search, applicant);
            if (value_select == ERROR)
            {
                return -1;
            }
        }
        else if (value->kind == RECORD_KIND_TYPE)
        {
            not_record_type_t *record_type = (not_record_type_t *)value;
            not_node_t *type = record_type->type;
            not_strip_t *strip_new = (not_strip_t *)record_type->value;
            value_select = not_execute_attribute_from_type(node, strip_new, type, key_search, applicant);
            if (value_select == ERROR)
            {
                return -1;
            }
        }
        else if (value->kind == RECORD_KIND_OBJECT)
        {
            for (not_record_object_t *object = value->value; object != NULL; object = object->next)
            {
                if (not_execute_id_strcmp(key_search, object->key) == 1)
                {
                    value_select = object->value;
                    break;
                }
            }
        }
    }

    if (!value_select)
    {
        if (entity->value)
        {
            value_select = not_execute_expression(entity->value, strip, applicant, NULL);
            if (value_select == ERROR)
            {
                return -1;
            }
        }
    }

    if (value_select)
    {
        if (value_select->link > 0)
        {
            if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
            {
                value_select = not_record_copy(value_select);
                if (value_select == ERROR)
                {
                    return -1;
                }
            }
        }
        else
        {
            if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
            {
                not_error_type_by_node(entity->key, "the unreferenced type is assigned to the reference type");
                return -1;
            }
        }

        if ((entity->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
        {
            value_select->readonly = 1;
        }
    }

    if (((entity->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC) || (scope->kind == NODE_KIND_MODULE))
    {
        not_entry_t *entry = not_symbol_table_push(scope, node, entity->key, value_select);
        if (entry == ERROR)
        {
            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(value_select) < 0)
                    {
                        return -1;
                    }
                }
            }
            return -1;
        }
        else if (entry == NULL)
        {
            not_node_basic_t *basic1 = (not_node_basic_t *)entity->key->value;
            not_error_type_by_node(entity->key, "'%s' already defined",
                                   basic1->value);

            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(value_select) < 0)
                    {
                        return -1;
                    }
                }
            }
            return -1;
        }
    }
    else
    {
        not_entry_t *entry = not_strip_variable_push(strip, scope, node, entity->key, value_select);
        if (entry == ERROR)
        {
            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(value_select) < 0)
                    {
                        return -1;
                    }
                }
            }

            return -1;
        }
        else if (entry == NULL)
        {
            not_node_basic_t *basic1 = (not_node_basic_t *)entity->key->value;
            not_error_type_by_node(entity->key, "'%s' already defined",
                                   basic1->value);

            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(value_select) < 0)
                    {
                        return -1;
                    }
                }
            }
            return -1;
        }
    }

    return 0;
}

static int32_t
not_execute_set(not_node_t *scope, not_node_t *node, not_node_t *value, not_strip_t *strip, not_node_t *applicant)
{
    not_record_t *record_value = not_execute_expression(value, strip, applicant, NULL);
    if (record_value == ERROR)
    {
        return -1;
    }

    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        if (not_execute_entity(scope, item, record_value, strip, applicant) < 0)
        {
            if (not_record_link_decrease(record_value) < 0)
            {
                return -1;
            }
            return -1;
        }
    }

    if (not_record_link_decrease(record_value) < 0)
    {
        return -1;
    }

    return 0;
}

static int32_t
not_execute_var(not_node_t *scope, not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_var_t *var1 = (not_node_var_t *)node->value;
    if (var1->key->kind == NODE_KIND_SET)
    {
        if (not_execute_set(scope, var1->key, var1->value, strip, applicant) < 0)
        {
            return -1;
        }
    }
    else
    {
        not_record_t *record_value = NULL;
        if (var1->value)
        {
            record_value = not_execute_expression(var1->value, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                return -1;
            }

            if (var1->type)
            {
                not_record_t *record_type = not_execute_expression(var1->type, strip, applicant, NULL);
                if (record_type == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return -1;
                    }

                    return -1;
                }

                if (record_type->kind != RECORD_KIND_TYPE)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;
                    not_error_type_by_node(var1->key, "'%s' unsupported type: '%s'",
                                           basic1->value, not_record_type_as_string(record_type));

                    if (not_record_link_decrease(record_type) < 0)
                    {
                        return -1;
                    }

                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return -1;
                    }

                    return -1;
                }

                int32_t r1 = not_execute_value_check_by_type(var1->key, record_value, record_type, strip, applicant);
                if (r1 < 0)
                {
                    if (not_record_link_decrease(record_type) < 0)
                    {
                        return -1;
                    }
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return -1;
                    }
                }
                else if (r1 == 0)
                {
                    if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;
                        not_error_type_by_node(var1->key, "'%s' mismatch: '%s' and '%s'",
                                               basic1->value, not_record_type_as_string(record_type), not_record_type_as_string(record_value));

                        if (not_record_link_decrease(record_type) < 0)
                        {
                            return -1;
                        }
                        if (not_record_link_decrease(record_value) < 0)
                        {
                            return -1;
                        }

                        return -1;
                    }
                    else
                    {
                        if (record_value->link > 1)
                        {
                            not_record_t *record_copy = not_record_copy(record_value);
                            if (not_record_link_decrease(record_value) < 0)
                            {
                                return -1;
                            }
                            record_value = record_copy;
                        }

                        not_record_t *record_value2 = not_execute_value_casting_by_type(var1->key, record_value, record_type, strip, applicant);
                        if (record_value2 == ERROR)
                        {
                            if (not_record_link_decrease(record_type) < 0)
                            {
                                return -1;
                            }
                            if (not_record_link_decrease(record_value) < 0)
                            {
                                return -1;
                            }

                            return -1;
                        }
                        else if (record_value2 == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;
                            not_error_type_by_node(var1->key, "'%s' mismatch: '%s' and '%s'",
                                                   basic1->value, not_record_type_as_string(record_type), not_record_type_as_string(record_value));

                            if (not_record_link_decrease(record_type) < 0)
                            {
                                return -1;
                            }
                            if (not_record_link_decrease(record_value) < 0)
                            {
                                return -1;
                            }

                            return -1;
                        }

                        record_value = record_value2;
                    }
                }

                if (not_record_link_decrease(record_type) < 0)
                {
                    return -1;
                }
            }
        }

        if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
        {
            if (record_value->link > 1)
            {
                not_record_t *record_copy = not_record_copy(record_value);
                if (not_record_link_decrease(record_value) < 0)
                {
                    return -1;
                }
                record_value = record_copy;
            }
        }

        if ((var1->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
        {
            record_value->readonly = 1;
        }

        if (var1->type)
        {
            record_value->typed = 1;
        }

        if (((var1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC) || (scope->kind == NODE_KIND_MODULE))
        {
            not_entry_t *entry = not_symbol_table_push(scope, node, var1->key, record_value);
            if (entry == ERROR)
            {
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return -1;
                    }
                }
                return -1;
            }
            else if (entry == NULL)
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;
                not_error_type_by_node(var1->key, "'%s' already defined",
                                       basic1->value);

                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return -1;
                    }
                }
                return -1;
            }
        }
        else
        {
            not_entry_t *entry = not_strip_variable_push(strip, scope, node, var1->key, record_value);
            if (entry == ERROR)
            {
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return -1;
                    }
                }
                return -1;
            }
            else if (entry == NULL)
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;
                not_error_type_by_node(var1->key, "'%s' already defined",
                                       basic1->value);

                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return -1;
                    }
                }
                return -1;
            }
        }
    }

    return 0;
}

static int32_t
not_execute_for(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_for_t *for1 = (not_node_for_t *)node->value;
    int32_t ret_code = 0;

    if (for1->initializer != NULL)
    {
        for (not_node_t *item1 = for1->initializer; item1 != NULL; item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_VAR)
            {
                int32_t r1 = not_execute_var(node, item1, strip, applicant);
                if (r1 == -1)
                {
                    return -1;
                }
            }
            else
            {
                int32_t r1 = not_execute_assign(item1, strip, applicant, NULL);
                if (r1 == -1)
                {
                    return -1;
                }
            }
        }
    }

    int32_t truthy = 1;

region_start_loop:
    if (for1->condition)
    {
        not_record_t *condition = not_execute_expression(for1->condition, strip, applicant, NULL);
        if (condition == ERROR)
        {
            goto region_error;
        }

        truthy = not_execute_truthy(condition);
        if (not_record_link_decrease(condition) < 0)
        {
            return -1;
        }
    }

    if (truthy)
    {
        if (for1->body)
        {
            int32_t r2 = not_execute_body(for1->body, strip, applicant);
            if (r2 == -1)
            {
                goto region_error;
            }
            else if (r2 == -2)
            {
                ret_code = -2;
                not_record_t *rax = not_thread_get_rax();
                if (rax == ERROR)
                {
                    goto region_error;
                }

                if (rax)
                {
                    if (for1->key)
                    {
                        not_record_type_t *type = (not_record_type_t *)rax->value;
                        if (type->type->kind == NODE_KIND_FOR)
                        {
                            not_node_for_t *for2 = (not_node_for_t *)type->type->value;
                            if (not_execute_id_cmp(for1->key, for2->key) == 1)
                            {
                                if (not_record_link_decrease(rax) < 0)
                                {
                                    goto region_error;
                                }
                                not_thread_set_rax(NULL);
                                ret_code = 0;
                            }
                        }
                    }

                    goto region_end_loop;
                }
                else
                {
                    ret_code = 0;
                    goto region_end_loop;
                }
            }
            else if (r2 == -3)
            {
                ret_code = -3;

                not_record_t *rax = not_thread_get_rax();
                if (rax == ERROR)
                {
                    goto region_error;
                }

                if (rax)
                {
                    if (for1->key)
                    {
                        not_record_type_t *type = (not_record_type_t *)rax->value;
                        if (type->type->kind == NODE_KIND_FOR)
                        {
                            not_node_for_t *for2 = (not_node_for_t *)type->type->value;
                            if (not_execute_id_cmp(for1->key, for2->key) == 1)
                            {
                                if (not_record_link_decrease(rax) < 0)
                                {
                                    goto region_error;
                                }
                                not_thread_set_rax(NULL);
                                ret_code = 0;

                                goto region_continue_loop;
                            }
                        }
                    }

                    goto region_end_loop;
                }
                else
                {
                    ret_code = 0;
                    goto region_continue_loop;
                }
            }
            else if (r2 < 0)
            {
                ret_code = r2;
                goto region_end_loop;
            }
        }

    region_continue_loop:
        if (for1->incrementor)
        {
            for (not_node_t *item1 = for1->incrementor; item1 != NULL; item1 = item1->next)
            {
                int32_t r1 = not_execute_assign(item1, strip, applicant, NULL);
                if (r1 == -1)
                {
                    goto region_error;
                }
            }
        }

        goto region_start_loop;
    }

region_end_loop:
    if (not_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }

    return ret_code;

region_error:
    return -1;
}

static int32_t
not_execute_forin(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_forin_t *for1 = (not_node_forin_t *)node->value;
    int32_t ret_code = 0;

    not_record_t *iterator = not_execute_expression(for1->iterator, strip, applicant, NULL);
    if (iterator == ERROR)
    {
        return -1;
    }

    not_record_object_t *object = NULL;
    not_record_tuple_t *tuple = NULL;
region_start_loop:
    if (iterator->kind == RECORD_KIND_OBJECT)
    {
        if (object)
        {
            object = object->next;
        }
        else
        {
            object = (not_record_object_t *)iterator->value;
        }

        if (object)
        {
            if (for1->value)
            {
                not_record_t *record_key = not_record_make_string(object->key);
                if (record_key == ERROR)
                {
                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }

                not_entry_t *entry = not_strip_variable_push(strip, node, node, for1->field, record_key);
                if (entry == ERROR)
                {
                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->field->value;
                    not_error_type_by_node(for1->field, "'%s' already defined", basic1->value);

                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }

                entry = not_strip_variable_push(strip, node, node, for1->value, object->value);
                if (entry == ERROR)
                {
                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->value->value;
                    not_error_type_by_node(for1->value, "'%s' already defined", basic1->value);

                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                not_record_link_increase(object->value);
            }
            else
            {
                not_entry_t *entry = not_strip_variable_push(strip, node, node, for1->field, object->value);
                if (entry == ERROR)
                {
                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->field->value;
                    not_error_type_by_node(for1->field, "'%s' already defined", basic1->value);

                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                not_record_link_increase(object->value);
            }
        }
        else
        {
            goto region_end_loop;
        }
    }
    else if (iterator->kind == RECORD_KIND_TUPLE)
    {
        if (tuple)
        {
            tuple = tuple->next;
        }
        else
        {
            tuple = (not_record_tuple_t *)iterator->value;
        }

        if (tuple)
        {
            if (for1->value)
            {
                not_record_t *record_key = not_record_make_undefined();
                if (record_key == ERROR)
                {
                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }

                not_entry_t *entry = not_strip_variable_push(strip, node, node, for1->field, record_key);
                if (entry == ERROR)
                {
                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->field->value;
                    not_error_type_by_node(for1->field, "'%s' already defined",
                                           basic1->value);

                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }

                entry = not_strip_variable_push(strip, node, node, for1->value, tuple->value);
                if (entry == ERROR)
                {
                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->value->value;
                    not_error_type_by_node(for1->value, "'%s' already defined",
                                           basic1->value);

                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                not_record_link_increase(tuple->value);
            }
            else
            {
                not_entry_t *entry = not_strip_variable_push(strip, node, node, for1->field, tuple->value);
                if (entry == ERROR)
                {
                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->field->value;
                    not_error_type_by_node(for1->field, "'%s' already defined",
                                           basic1->value);

                    if (not_record_link_decrease(iterator) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }
                not_record_link_increase(tuple->value);
            }
        }
        else
        {
            goto region_end_loop;
        }
    }

    int32_t r2 = not_execute_body(for1->body, strip, applicant);
    if (r2 == -1)
    {
        goto region_error;
    }
    else if (r2 == -2)
    {
        ret_code = -2;
        not_record_t *rax = not_thread_get_rax();
        if (rax == ERROR)
        {
            goto region_error;
        }

        if (rax)
        {
            if (for1->key)
            {
                not_record_type_t *type = (not_record_type_t *)rax->value;
                if (type->type->kind == NODE_KIND_FORIN)
                {
                    not_node_forin_t *for2 = (not_node_forin_t *)type->type->value;
                    if (not_execute_id_cmp(for1->key, for2->key) == 1)
                    {
                        if (not_record_link_decrease(rax) < 0)
                        {
                            goto region_error;
                        }
                        not_thread_set_rax(NULL);
                        ret_code = 0;
                    }
                }
            }

            goto region_end_loop;
        }
        else
        {
            ret_code = 0;
            goto region_end_loop;
        }
    }
    else if (r2 == -3)
    {
        ret_code = -3;

        not_record_t *rax = not_thread_get_rax();
        if (rax == ERROR)
        {
            goto region_error;
        }

        if (rax)
        {
            if (for1->key)
            {
                not_record_type_t *type = (not_record_type_t *)rax->value;
                if (type->type->kind == NODE_KIND_FORIN)
                {
                    not_node_forin_t *for2 = (not_node_forin_t *)type->type->value;
                    if (not_execute_id_cmp(for1->key, for2->key) == 1)
                    {
                        if (not_record_link_decrease(rax) < 0)
                        {
                            goto region_error;
                        }
                        not_thread_set_rax(NULL);
                        ret_code = 0;

                        goto region_continue_loop;
                    }
                }
            }

            goto region_end_loop;
        }
        else
        {
            ret_code = 0;
            goto region_continue_loop;
        }
    }
    else if (r2 < 0)
    {
        ret_code = r2;
        goto region_end_loop;
    }

region_continue_loop:
    if (not_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }
    goto region_start_loop;

region_end_loop:

    if (not_record_link_decrease(iterator) < 0)
    {
        return -1;
    }

    if (not_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }

    return ret_code;

region_error:
    return -1;
}

static int32_t
not_execute_try(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_try_t *try1 = (not_node_try_t *)node->value;

    int32_t r2 = not_execute_body(try1->body, strip, applicant);
    if ((r2 == -4) || (r2 == -1))
    {
        if (r2 == -1)
        {
            if (!not_config_expection_is_enable())
            {
                return -1;
            }
        }

        not_thread_t *t = not_thread_get_current();

        for (not_queue_entry_t *a = t->interpreter->expections->begin, *b = NULL; a != t->interpreter->expections->end; a = b)
        {
            b = a->next;
            not_record_t *expection = (not_record_t *)a->value;

            if (try1->catchs)
            {
                not_node_catch_t *catch1 = (not_node_catch_t *)try1->catchs->value;
                int32_t r1 = not_call_parameters_check_by_one_arg(try1->catchs, strip, catch1->parameters, expection, applicant);
                if (r1 == -1)
                {
                    return -1;
                }
                else if (r1 == 0)
                {
                    continue;
                }

                if (not_call_parameters_subs_by_one_arg(try1->catchs, try1->catchs, strip, catch1->parameters, expection, applicant) < 0)
                {
                    return -1;
                }

                if (not_record_link_decrease(expection) < 0)
                {
                    return -1;
                }

                not_queue_unlink(t->interpreter->expections, a);

                if (catch1->parameters)
                {
                    not_node_block_t *parameters = (not_node_block_t *)catch1->parameters->value;
                    for (not_node_t *item = parameters->items; item != NULL; item = item->next)
                    {
                        not_node_parameter_t *parameter = (not_node_parameter_t *)item->value;
                        not_entry_t *entry = not_strip_input_find(strip, try1->catchs, parameter->key);
                        if (entry == ERROR)
                        {
                            return -1;
                        }

                        not_entry_t *entry2 = not_strip_variable_push(strip, try1->catchs, item, parameter->key, entry->value);
                        if (entry2 == ERROR)
                        {
                            return -1;
                        }
                    }
                }

                int32_t r3 = not_execute_body(catch1->body, strip, applicant);
                if (r3 < 0)
                {
                    return r3;
                }

                if (not_strip_variable_remove_by_scope(strip, try1->catchs) < 0)
                {
                    return -1;
                }
            }
            else
            {
                continue;
            }
        }

        if (not_queue_count(t->interpreter->expections) > 0)
        {
            return r2;
        }
    }
    else if (r2 < 0)
    {
        return r2;
    }

    return 0;
}

static int32_t
not_execute_if(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_if_t *if1 = (not_node_if_t *)node->value;
    if (if1->condition != NULL)
    {
        not_record_t *condition = not_execute_expression(if1->condition, strip, applicant, NULL);
        if (condition == ERROR)
        {
            return -1;
        }

        int32_t truthy = not_execute_truthy(condition);
        if (not_record_link_decrease(condition) < 0)
        {
            return -1;
        }

        if (!truthy)
        {
            goto region_else;
        }
    }

    if (if1->then_body != NULL)
    {
        int32_t r1 = not_execute_body(if1->then_body, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }

    return 0;

region_else:
    if (if1->else_body != NULL)
    {
        not_node_t *else_body1 = if1->else_body;

        if (else_body1->kind == NODE_KIND_IF)
        {
            int32_t r1 = not_execute_if(if1->else_body, strip, applicant);
            if (r1 < 0)
            {
                return r1;
            }
        }
        else
        {
            int32_t r1 = not_execute_body(if1->else_body, strip, applicant);
            if (r1 < 0)
            {
                return r1;
            }
        }
    }

    return 0;
}

static int32_t
not_execute_throw(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_unary_t *unary = (not_node_unary_t *)node->value;

    if (unary->right)
    {
        not_record_t *value = not_execute_expression(unary->right, strip, applicant, NULL);
        if (value == ERROR)
        {
            return -1;
        }

        not_thread_t *t = not_thread_get_current();
        if (ERROR == not_queue_right_push(t->interpreter->expections, value))
        {
            return -1;
        }
    }

    return -4;
}

static int32_t
not_execute_break(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_unary_t *unary = (not_node_unary_t *)node->value;

    if (unary->right)
    {
        not_record_t *value = not_execute_expression(unary->right, strip, applicant, NULL);
        if (value == ERROR)
        {
            return -1;
        }

        if (value->kind != RECORD_KIND_TYPE)
        {
            if (not_record_link_decrease(value) < 0)
            {
                return -1;
            }
            goto region_error;
        }

        not_record_type_t *type = (not_record_type_t *)value->value;
        if ((type->type->kind != NODE_KIND_FOR) && (type->type->kind != NODE_KIND_FORIN))
        {
            if (not_record_link_decrease(value) < 0)
            {
                return -1;
            }
            goto region_error;
        }

        not_thread_set_rax(value);
    }

    return -2;

region_error:
    not_error_type_by_node(unary->right, "for loop is not mentioned");
    return -1;
}

static int32_t
not_execute_continue(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_unary_t *unary = (not_node_unary_t *)node->value;

    if (unary->right)
    {
        not_record_t *value = not_execute_expression(unary->right, strip, applicant, NULL);
        if (value == ERROR)
        {
            return -1;
        }

        if (value->kind != RECORD_KIND_TYPE)
        {
            if (not_record_link_decrease(value) < 0)
            {
                return -1;
            }
            goto region_error;
        }

        not_record_type_t *type = (not_record_type_t *)value->value;
        if ((type->type->kind != NODE_KIND_FOR) && (type->type->kind != NODE_KIND_FORIN))
        {
            if (not_record_link_decrease(value) < 0)
            {
                return -1;
            }
            goto region_error;
        }

        not_thread_set_rax(value);
    }

    return -3;

region_error:
    not_error_type_by_node(unary->right, "for loop is not mentioned");
    return -1;
}

static int32_t
not_execute_return(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_unary_t *unary = (not_node_unary_t *)node->value;

    if (unary->right)
    {
        not_record_t *value = not_execute_expression(unary->right, strip, applicant, NULL);
        if (value == ERROR)
        {
            return -1;
        }

        not_thread_set_rax(value);
    }

    return 0;
}

static int32_t
not_execute_statement(not_node_t *scope, not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t r1 = not_execute_var(scope, node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else if (node->kind == NODE_KIND_FOR)
    {
        int32_t r1 = not_execute_for(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else if (node->kind == NODE_KIND_FORIN)
    {
        int32_t r1 = not_execute_forin(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else if (node->kind == NODE_KIND_IF)
    {
        int32_t r1 = not_execute_if(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else if (node->kind == NODE_KIND_TRY)
    {
        int32_t r1 = not_execute_try(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else if (node->kind == NODE_KIND_BREAK)
    {
        int32_t r1 = not_execute_break(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else if (node->kind == NODE_KIND_CONTINUE)
    {
        int32_t r1 = not_execute_continue(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else if (node->kind == NODE_KIND_RETURN)
    {
        int32_t r1 = not_execute_return(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else if (node->kind == NODE_KIND_THROW)
    {
        int32_t r1 = not_execute_throw(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else
    {
        int32_t r1 = not_execute_assign(node, strip, applicant, NULL);
        if (r1 < 0)
        {
            return r1;
        }
    }

    return 0;
}

static int32_t
not_execute_body(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        int32_t r1 = not_execute_statement(node, item, strip, applicant);
        if (r1 == -1)
        {
            if (not_strip_variable_remove_by_scope(strip, node) < 0)
            {
                return -1;
            }

            return -1;
        }
        else if (r1 < 0)
        {
            if (not_strip_variable_remove_by_scope(strip, node) < 0)
            {
                return -1;
            }
            return r1;
        }
    }

    if (not_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }

    return 0;
}

int32_t
not_execute_run_fun(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_fun_t *fun = (not_node_fun_t *)node->value;

    if (fun->parameters)
    {
        not_node_block_t *parameters = (not_node_block_t *)fun->parameters->value;
        for (not_node_t *item = parameters->items; item != NULL; item = item->next)
        {
            not_node_parameter_t *parameter = (not_node_parameter_t *)item->value;
            not_entry_t *entry = not_strip_input_find(strip, node, parameter->key);
            if (entry == ERROR)
            {
                return -1;
            }

            not_entry_t *entry2 = not_strip_variable_push(strip, node, item, parameter->key, entry->value);
            if (entry2 == ERROR)
            {
                return -1;
            }
        }
    }

    int32_t r1 = not_execute_body(fun->body, strip, applicant);

    if (not_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }

    if (r1 == -1)
    {
        return -1;
    }
    else if (r1 < 0)
    {
        if (r1 == -4)
        {
            return r1;
        }
        not_error_runtime_by_node(node, "there is no loop");
        return -1;
    }

    return 0;
}

int32_t
not_execute_run_lambda(not_node_t *node, not_strip_t *strip, not_node_t *applicant)
{
    not_node_lambda_t *fun = (not_node_lambda_t *)node->value;

    if (fun->parameters)
    {
        not_node_block_t *parameters = (not_node_block_t *)fun->parameters->value;
        for (not_node_t *item = parameters->items; item != NULL; item = item->next)
        {
            not_node_parameter_t *parameter = (not_node_parameter_t *)item->value;
            not_entry_t *entry = not_strip_input_find(strip, node, parameter->key);
            if (entry == ERROR)
            {
                return -1;
            }

            not_entry_t *entry2 = not_strip_variable_push(strip, node, item, parameter->key, entry->value);
            if (entry2 == ERROR)
            {
                return -1;
            }
        }
    }

    int32_t r1 = not_execute_body(fun->body, strip, applicant);

    if (not_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }

    if (r1 == -1)
    {
        return -1;
    }
    else if (r1 < 0)
    {
        if (r1 == -4)
        {
            return r1;
        }
        not_error_runtime_by_node(node, "there is no loop");
        return -1;
    }

    return 0;
}

static int32_t
not_execute_module(not_node_t *node)
{
    not_strip_t *strip = not_strip_create(NULL);
    if (strip == ERROR)
    {
        return -1;
    }

    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_CLASS)
        {
            continue;
        }
        else if (item->kind == NODE_KIND_USING)
        {
            continue;
        }
        else
        {
            int32_t r = not_execute_statement(node, item, strip, node);
            if (r < 0)
            {
                if (r == -4)
                {
                    not_error_runtime_by_node(node, "expection is not handled");
                }
                return -1;
            }
        }
    }

    if (not_strip_destroy(strip) < 0)
    {
        return -1;
    }

    return 0;
}

int32_t
not_execute_run(not_node_t *root)
{
    if (not_execute_module(root) < 0)
    {
        return -1;
    }
    return 0;
}
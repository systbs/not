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
#include "../../memory.h"
#include "../../interpreter.h"
#include "../../thread.h"
#include "../record.h"
#include "../garbage.h"
#include "../entry.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

int32_t
sy_execute_id_cmp(sy_node_t *n1, sy_node_t *n2)
{
	sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;
	sy_node_basic_t *nb2 = (sy_node_basic_t *)n2->value;

	return (strcmp(nb1->value, nb2->value) == 0);
}

int32_t
sy_execute_id_strcmp(sy_node_t *n1, const char *name)
{
    sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;

	return (strcmp(nb1->value, name) == 0);
}

static int32_t 
sy_execute_body(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant);

int32_t
sy_execute_truthy(sy_record_t *left)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT)
    {
        return !(mpz_cmp_si((*(mpz_t *)(left->value)), 0) == 0);
    }
    else
    if (left->kind == RECORD_KIND_FLOAT)
    {
        return !(mpf_cmp_d((*(mpf_t *)(left->value)), 0.0) == 0);
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        return !((*(char *)(left->value)) == 0);
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        return 1;
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        return 1;
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        return 1;
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        return 1;
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        return 1;
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        return 0;
    }
    return 0;
}

int32_t
sy_execute_type_check_by_type(sy_record_t *record_type1, sy_record_t *record_type2, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_record_type_t *type1 = (sy_record_type_t *)record_type1->value;
	sy_record_type_t *type2 = (sy_record_type_t *)record_type2->value;

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
	else
	if (type1->type->kind == NODE_KIND_KFLOAT)
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
	else
	if (type1->type->kind == NODE_KIND_OBJECT)
	{
		if (type2->type->kind == RECORD_KIND_OBJECT)
		{
            uint64_t cnt1 = 0;
            for (sy_record_object_t *object1 = (sy_record_object_t *)type1->value;object1 != NULL;object1 = object1->next)
            {
                int32_t found = 0;
                cnt1 += 1;
                for (sy_record_object_t *object2 = (sy_record_object_t *)type2->value;object2 != NULL;object2 = object2->next)
                {
                    if (sy_execute_id_cmp(object1->key, object2->key) == 1)
					{
                        int32_t r1 = sy_execute_type_check_by_type(object1->value, object2->value, strip, applicant);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
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
            for (sy_record_object_t *object2 = (sy_record_object_t *)type2->value;object2 != NULL;object2 = object2->next)
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
	else
	if (type1->type->kind == NODE_KIND_ARRAY)
	{
        if (type2->type->kind == NODE_KIND_ARRAY)
	    {
            int32_t r1 = sy_execute_type_check_by_type(type1->value, type2->value, strip, applicant);
            if (r1 == -1)
            {
                return -1;
            }
            else
            if (r1 == 0)
            {
                return 0;
            }
            return 1;
        }
        return 0;
	}
	else
	if (type1->type->kind == NODE_KIND_TUPLE)
	{
		if (type2->type->kind == NODE_KIND_TUPLE)
		{
            uint64_t cnt1 = 0;
            for (
                sy_record_tuple_t *tuple1 = (sy_record_tuple_t *)type1->value, *tuple2 = (sy_record_tuple_t *)type2->value;
                tuple1 != NULL;
                tuple1 = tuple1->next, tuple2 = tuple2->next)
            {
                cnt1 += 1;

                if (tuple2 == NULL)
                {
                    return 0;
                }

                int32_t r1 = sy_execute_type_check_by_type(tuple2->value, tuple1->value, strip, applicant);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 0)
                {
                    return 0;
                }
            }

            uint64_t cnt2 = 0;
            for (sy_record_tuple_t *tuple2 = (sy_record_tuple_t *)type2->value;tuple2 != NULL;tuple2 = tuple2->next)
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
	else
	if (type1->type->kind == NODE_KIND_CLASS)
	{
		if (type2->type->kind == NODE_KIND_CLASS)
		{
			if (type1->type->id != type2->type->id)
			{
				return 0;
			}

			sy_node_class_t *class1 = (sy_node_class_t *)type1->type->value;
            if (class1->generics)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)class1->generics->value;
                for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                {
                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;

                    sy_entry_t *strip_entry1 = sy_strip_variable_find(type1->value, type1->type, generic1->key);
                    if (strip_entry1 == ERROR)
                    {
                        return -1;
                    }
                    if (strip_entry1 == NULL)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                        sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                        return -1;
                    }

                    sy_entry_t *strip_entry2 = sy_strip_variable_find(type2->value, type2->type, generic1->key);
                    if (strip_entry2 == ERROR)
                    {
                        strip_entry1->value->link -= 1;
                        return -1;
                    }
                    if (strip_entry2 == NULL)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                        sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                        strip_entry1->value->link -= 1;
                        return -1;
                    }

                    int32_t r1 = sy_execute_type_check_by_type(strip_entry1->value, strip_entry2->value, strip, applicant);
                    if (r1 == -1)
                    {
                        strip_entry1->value->link -= 1;
                        strip_entry2->value->link -= 1;
                        return -1;
                    }
                    else
                    if (r1 == 0)
                    {
                        strip_entry1->value->link -= 1;
                        strip_entry2->value->link -= 1;
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
sy_execute_value_check_by_type(sy_record_t *record_value, sy_record_t *record_type, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_record_type_t *type1 = (sy_record_type_t *)record_type->value;

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
	else
	if (type1->type->kind == NODE_KIND_KFLOAT)
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
	else
	if (type1->type->kind == NODE_KIND_OBJECT)
	{
		if (record_value->kind == RECORD_KIND_OBJECT)
		{
            uint64_t cnt1 = 0;
            for (sy_record_object_t *object1 = (sy_record_object_t *)record_value->value;object1 != NULL;object1 = object1->next)
            {
                int32_t found = 0;
                cnt1 += 1;
                for (sy_record_object_t *object2 = (sy_record_object_t *)type1->value;object2 != NULL;object2 = object2->next)
                {
                    if (sy_execute_id_cmp(object1->key, object2->key) == 1)
					{
                        int32_t r1 = sy_execute_value_check_by_type(object1->value, object2->value, strip, applicant);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
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
            for (sy_record_object_t *object2 = (sy_record_object_t *)type1->value;object2 != NULL;object2 = object2->next)
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
	else
	if (type1->type->kind == NODE_KIND_ARRAY)
	{
        if (record_value->kind == RECORD_KIND_TUPLE)
		{
            for (sy_record_tuple_t *tuple = (sy_record_tuple_t *)record_value->value;tuple != NULL;tuple = tuple->next)
            {
                int32_t r1 = sy_execute_value_check_by_type(tuple->value, (sy_record_t *)type1->value, strip, applicant);
                if (r1 == -1)
				{
					return -1;
				}
                else
				if (r1 == 0)
				{
					return 0;
				}
            }

            return 1;
		}

		return 0;
	}
	else
	if (type1->type->kind == NODE_KIND_TUPLE)
	{
        if (record_value->kind == RECORD_KIND_TUPLE)
		{
            uint64_t cnt1 = 0;
            for (
                sy_record_tuple_t *tuple1 = (sy_record_tuple_t *)type1->value, *tuple2 = (sy_record_tuple_t *)record_value->value;
                tuple1 != NULL;
                tuple1 = tuple1->next, tuple2 = tuple2->next)
            {
                cnt1 += 1;

                if (tuple2 == NULL)
                {
                    return 0;
                }

                int32_t r1 = sy_execute_value_check_by_type(tuple2->value, tuple1->value, strip, applicant);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 0)
                {
                    return 0;
                }
            }

            uint64_t cnt2 = 0;
            for (sy_record_tuple_t *tuple2 = (sy_record_tuple_t *)record_value->value;tuple2 != NULL;tuple2 = tuple2->next)
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
	else
	if (type1->type->kind == NODE_KIND_CLASS)
	{
		if (record_value->kind == RECORD_KIND_STRUCT)
		{
			sy_record_struct_t *struct1 = (sy_record_struct_t *)record_value->value;
			if (type1->type->id != struct1->type->id)
			{
				return 0;
			}

			sy_node_class_t *class1 = (sy_node_class_t *)struct1->type->value;
            if (class1->generics)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)class1->generics->value;
                if (block1 != NULL)
                {
                    for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                    {
                        sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;

                        sy_entry_t *strip_entry1 = sy_strip_variable_find(struct1->value, struct1->type, generic1->key);
                        if (strip_entry1 == ERROR)
                        {
                            return -1;
                        }
                        if (strip_entry1 == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                            sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                            return -1;
                        }

                        sy_entry_t *strip_entry2 = sy_strip_variable_find(type1->value, type1->type, generic1->key);
                        if (strip_entry2 == ERROR)
                        {
                            strip_entry1->value->link -= 1;
                            return -1;
                        }
                        if (strip_entry2 == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                            sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                            strip_entry1->value->link -= 1;
                            return -1;
                        }

                        int32_t r1 = sy_execute_type_check_by_type(strip_entry1->value, strip_entry2->value, strip, applicant);
                        if (r1 == -1)
                        {
                            strip_entry1->value->link -= 1;
                            strip_entry2->value->link -= 1;
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            strip_entry1->value->link -= 1;
                            strip_entry2->value->link -= 1;
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

sy_record_t *
sy_execute_value_casting_by_type(sy_record_t *record_value, sy_record_t *record_type, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_record_type_t *type1 = (sy_record_type_t *)record_type->value;

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
	else
	if (type1->type->kind == NODE_KIND_KFLOAT)
	{
		if (record_value->kind == RECORD_KIND_FLOAT)
		{
			return record_value;
		}
        else
        if (record_value->kind == RECORD_KIND_INT)
		{
            mpz_t val;
            mpz_init_set(val, *(mpz_t *)(record_value->value));
            void *ptr = sy_memory_realloc(record_value->value, sizeof(mpf_t));
            if (!ptr)
            {
                sy_error_no_memory();
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
	else
	if (type1->type->kind == NODE_KIND_OBJECT)
	{
		if (record_value->kind == RECORD_KIND_OBJECT)
		{
            sy_record_object_t *object_copy = sy_record_object_copy((sy_record_object_t *)record_value->value);
            if (object_copy == ERROR)
            {
                return ERROR;
            }

            uint64_t cnt1 = 0;
            for (sy_record_object_t *object1 = (sy_record_object_t *)type1->value; object1 != NULL;object1 = object1->next)
            {
                cnt1 += 1;
                int32_t found = 0;
                for (sy_record_object_t *object2 = object_copy; object2 != NULL;object2 = object2->next)
                {
                    if (sy_execute_id_cmp(object1->key, object2->key) == 1)
					{
                        sy_record_t *r1 = sy_execute_value_casting_by_type(object2->value, object1->value, strip, applicant);
                        if (r1 == ERROR)
                        {
                            if (sy_record_object_destroy(object_copy) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }
                        else
                        if ((r1 == NULL) && (r1 != object2->value))
                        {
                            if (sy_record_object_destroy(object_copy) < 0)
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
                    if (sy_record_object_destroy(object_copy) < 0)
                    {
                        return ERROR;
                    }
                    return NULL;
                }
            }

            uint64_t cnt2 = 0;
            for (sy_record_object_t *object2 = object_copy; object2 != NULL;object2 = object2->next)
            {
                cnt2 += 1;
            }

            if (cnt1 != cnt2)
            {
                if (sy_record_object_destroy(object_copy) < 0)
                {
                    return ERROR;
                }
                return NULL;
            }

            if (sy_record_object_destroy(record_value->value) < 0)
            {
                if (sy_record_object_destroy(object_copy) < 0)
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
	else
	if (type1->type->kind == NODE_KIND_ARRAY)
	{
		if (record_value->kind == RECORD_KIND_TUPLE)
		{
            sy_record_tuple_t *tuple_copy = sy_record_tuple_copy((sy_record_tuple_t *)record_value->value);
			sy_record_t *array_type = (sy_record_t *)type1->value;

            for (sy_record_tuple_t *tuple = tuple_copy;tuple != NULL;tuple = tuple->next)
            {
                sy_record_t *r1 = sy_execute_value_casting_by_type(tuple->value, array_type, strip, applicant);
                if (r1 == ERROR)
				{
                    if (sy_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }
                else
                if ((r1 == NULL) && (r1 != tuple->value))
                {
                    if (sy_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return NULL;
                }

                tuple->value = r1;
            }

            if (sy_record_tuple_destroy(record_value->value) < 0)
            {
                return ERROR;
            }

            record_value->value = tuple_copy;

			return record_value;
		}

		return NULL;
	}
	else
	if (type1->type->kind == NODE_KIND_TUPLE)
	{
		if (record_value->kind == RECORD_KIND_TUPLE)
		{
			sy_record_tuple_t *tuple_copy = sy_record_tuple_copy((sy_record_tuple_t *)record_value->value);

            uint64_t cnt1 = 0;
            for (sy_record_tuple_t *tuple1 = (sy_record_tuple_t *)type1->value, *tuple2 = tuple_copy;tuple1 != NULL;tuple1 = tuple1->next, tuple2 = tuple2->next)
            {
                cnt1 += 1;

                if (tuple2 == NULL)
                {
                    if (sy_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return NULL;
                }

                sy_record_t *r1 = sy_execute_value_casting_by_type(tuple2->value, tuple1->value, strip, applicant);
                if (r1 == ERROR)
				{
                    if (sy_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }
                else
                if ((r1 == NULL) && (r1 != tuple2->value))
                {
                    if (sy_record_tuple_destroy(tuple_copy) < 0)
                    {
                        return ERROR;
                    }
                    return NULL;
                }

                tuple2->value = r1;
            }

            uint64_t cnt2 = 0;
            for (sy_record_tuple_t *tuple2 = tuple_copy;tuple2 != NULL;tuple2 = tuple2->next)
            {
                cnt2 += 1;
            }

            if (cnt1 != cnt2)
            {
                if (sy_record_tuple_destroy(tuple_copy) < 0)
                {
                    return ERROR;
                }
                return NULL;
            }

			if (sy_record_tuple_destroy(record_value->value) < 0)
            {
                return ERROR;
            }

            record_value->value = tuple_copy;

			return record_value;
		}

		return NULL;
	}
	else
	if (type1->type->kind == NODE_KIND_CLASS)
	{
		if (record_value->kind == RECORD_KIND_STRUCT)
		{
			sy_record_struct_t *struct1 = (sy_record_struct_t *)record_value->value;
			if (type1->type->id != struct1->type->id)
			{
				return NULL;
			}

            sy_node_class_t *class1 = (sy_node_class_t *)struct1->type->value;
            if (class1->generics)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)class1->generics->value;
                if (block1 != NULL)
                {
                    for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                    {
                        sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;

                        sy_entry_t *strip_entry1 = sy_strip_variable_find(struct1->value, struct1->type, generic1->key);
                        if (strip_entry1 == ERROR)
                        {
                            return ERROR;
                        }
                        if (strip_entry1 == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                            sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                            strip_entry1->value->link -= 1;
                            return ERROR;
                        }

                        sy_entry_t *strip_entry2 = sy_strip_variable_find(type1->value, type1->type, generic1->key);
                        if (strip_entry2 == ERROR)
                        {
                            strip_entry1->value->link -= 1;
                            return ERROR;
                        }
                        if (strip_entry2 == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                            sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                            strip_entry1->value->link -= 1;
                            return ERROR;
                        }

                        int32_t r1 = sy_execute_type_check_by_type(strip_entry1->value, strip_entry2->value, strip, applicant);
                        if (r1 == -1)
                        {
                            strip_entry2->value->link -= 1;
                            strip_entry1->value->link -= 1;
                            return ERROR;
                        }
                        else
                        if (r1 == 0)
                        {
                            strip_entry2->value->link -= 1;
                            strip_entry1->value->link -= 1;
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
sy_execute_entity(sy_node_t *scope, sy_node_t *node, sy_record_t *value, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_entity_t *entity = (sy_node_entity_t *)node->value;
    sy_node_t *key_search = entity->key;
    if (entity->type)
    {
        key_search = entity->type;
    }

    sy_record_t *value_select = NULL;

    if (value)
    {
        if (value->kind == RECORD_KIND_STRUCT)
        {
            sy_record_struct_t *record_struct = (sy_record_struct_t *)value->value;
            sy_node_t *type = record_struct->type;
            sy_strip_t *strip_new = (sy_strip_t *)record_struct->value;
            value_select = sy_execute_attribute_from_struct(node, strip_new, type, key_search, applicant);
            if (value_select == ERROR)
            {
                return -1;
            }
        }
        else
        if (value->kind == RECORD_KIND_TYPE)
        {
            sy_record_type_t *record_type = (sy_record_type_t *)value;
            sy_node_t *type = record_type->type;
            sy_strip_t *strip_new = (sy_strip_t *)record_type->value;
            value_select = sy_execute_attribute_from_type(node, strip_new, type, key_search, applicant);
            if (value_select == ERROR)
            {
                return -1;
            }
        }
        else
        if (value->kind == RECORD_KIND_OBJECT)
        {
            for (sy_record_object_t *object = value->value;object != NULL;object = object->next)
            {
                if (sy_execute_id_cmp(object->key, key_search) == 1)
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
            value_select = sy_execute_expression(entity->value, strip, applicant, NULL);
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
                value_select = sy_record_copy(value_select);
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
                sy_error_type_by_node(entity->key, "the unreferenced type is assigned to the reference type");
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
        sy_entry_t *entry = sy_symbol_table_push(scope, node, entity->key, value_select);
        if (entry == ERROR)
        {
            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    value_select->link -= 1;
                }
            }
            return -1;
        }
        else
        if (entry == NULL)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)entity->key->value;
            sy_error_type_by_node(entity->key, "'%s' already defined", 
                basic1->value);

            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    value_select->link -= 1;
                }
            }
            return -1;
        }
    }
    else
    {
        sy_entry_t *entry = sy_strip_variable_push(strip, scope, node, entity->key, value_select);
        if (entry == ERROR)
        {
            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    value_select->link -= 1;
                }
            }

            return -1;
        }
        else
        if (entry == NULL)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)entity->key->value;
            sy_error_type_by_node(entity->key, "'%s' already defined", 
                basic1->value);

            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    value_select->link -= 1;
                }
            }
            return -1;
        }
    }

    return 0;
}

static int32_t 
sy_execute_set(sy_node_t *scope, sy_node_t *node, sy_node_t *value, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_record_t *record_value = sy_execute_expression(value, strip, applicant, NULL);
    if (record_value == ERROR)
    {
        return -1;
    }

    sy_node_block_t *block = (sy_node_block_t *)node->value;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        if (sy_execute_entity(scope, item, record_value, strip, applicant) < 0)
        {
            record_value->link -= 1;
            return -1;
        }
    }

    record_value->link -= 1;

    return 0;
}

static int32_t 
sy_execute_var(sy_node_t *scope, sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_var_t *var1 = (sy_node_var_t *)node->value;
    if (var1->key->kind == NODE_KIND_SET)
	{
		if (sy_execute_set(scope, var1->key, var1->value, strip, applicant) < 0)
		{
			return -1;
		}
	}
    else
    {
        sy_record_t *record_value = NULL;
        if (var1->value)
        {
            record_value = sy_execute_expression(var1->value, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                return -1;
            }

            if (var1->type)
            {
                sy_record_t *record_type = sy_execute_expression(var1->type, strip, applicant, NULL);
                if (record_type == ERROR)
                {
                    record_value->link -= 1;

                    return -1;
                }

                if (record_type->kind != RECORD_KIND_TYPE)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;
                    sy_error_type_by_node(var1->key, "'%s' unsupported type: '%s'", 
                        basic1->value, sy_record_type_as_string(record_type));
                    
                    record_type->link -= 1;

                    record_value->link -= 1;

                    return -1;
                }

                int32_t r1 = sy_execute_value_check_by_type(record_value, record_type, strip, applicant);
                if (r1 < 0)
                {
                    record_type->link -= 1;
                    record_value->link -= 1;
                }
                else
                if (r1 == 0)
                {
                    if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;
                        sy_error_type_by_node(var1->key, "'%s' mismatch: '%s' and '%s'", 
                            basic1->value, sy_record_type_as_string(record_type), sy_record_type_as_string(record_value));

                        record_type->link -= 1;
                        record_value->link -= 1;

                        return -1;
                    }
                    else
                    {
                        if (record_value->link > 1)
                        {
                            sy_record_t *record_copy = sy_record_copy(record_value);
                            record_value->link -= 1;
                            record_value = record_copy;
                        }

                        sy_record_t *record_value2 = sy_execute_value_casting_by_type(record_value, record_type, strip, applicant);
                        if (record_value2 == ERROR)
                        {
                            record_type->link -= 1;
                            record_value->link -= 1;

                            return -1;
                        }
                        else
                        if (record_value2 == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;
                            sy_error_type_by_node(var1->key, "'%s' mismatch: '%s' and '%s'", 
                                basic1->value, sy_record_type_as_string(record_type), sy_record_type_as_string(record_value));

                            record_type->link -= 1;
                            record_value->link -= 1;
                            
                            return -1;
                        }

                        record_value = record_value2;
                    }
                }

                record_type->link -= 1;
            }
        }

        if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
        {
            if (record_value->link > 1)
            {
                sy_record_t *record_copy = sy_record_copy(record_value);
                record_value->link -= 1;
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
            sy_entry_t *entry = sy_symbol_table_push(scope, node, var1->key, record_value);
            if (entry == ERROR)
            {
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    record_value->link -= 1;
                }
                return -1;
            }
            else
            if (entry == NULL)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;
                sy_error_type_by_node(var1->key, "'%s' already defined", 
                    basic1->value);

                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    record_value->link -= 1;
                }
                return -1;
            }
        }
        else
        {
            sy_entry_t *entry = sy_strip_variable_push(strip, scope, node, var1->key, record_value);
            if (entry == ERROR)
            {
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    record_value->link -= 1;
                }
                return -1;
            }
            else
            if (entry == NULL)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;
                sy_error_type_by_node(var1->key, "'%s' already defined", 
                    basic1->value);

                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    record_value->link -= 1;
                }
                return -1;
            }
        }
    }

    return 0;
}

static int32_t 
sy_execute_for(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_for_t *for1 = (sy_node_for_t *)node->value;
    int32_t ret_code = 0;

    if (for1->initializer != NULL)
    {
        for (sy_node_t *item1 = for1->initializer;item1 != NULL;item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_VAR)
            {
                int32_t r1 = sy_execute_var(node, item1, strip, applicant);
                if (r1 == -1)
                {
                    return -1;
                }
            }
            else
            {
                int32_t r1 = sy_execute_assign(item1, strip, applicant, NULL);
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
        sy_record_t *condition = sy_execute_expression(for1->condition, strip, applicant, NULL);
        if (condition == ERROR)
        {
            goto region_error;
        }

        truthy = sy_execute_truthy(condition);
        condition->link -= 1;
    }

    if (truthy)
    {
        if (for1->body)
        {
            int32_t r2 = sy_execute_body(for1->body, strip, applicant);
            if (r2 == -1)
            {
                goto region_error;
            }
            else
            if (r2 == -2)
            {
                ret_code = -2;
                sy_record_t *rax = sy_thread_get_rax();
                if (rax == ERROR)
                {
                    goto region_error;
                }

                if (rax)
                {
                    if (for1->key)
                    {
                        sy_record_type_t *type = (sy_record_type_t *)rax->value;
                        sy_node_for_t *for2 = (sy_node_for_t *)type->type->value;
                        if (sy_execute_id_cmp(for1->key, for2->key) == 1)
                        {
                            rax->link -= 1;
                            if (sy_thread_set_rax(NULL) < 0)
                            {
                                goto region_error;
                            }
                            ret_code = 0;
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
            else
            if (r2 == -3)
            {
                ret_code = -3;

                sy_record_t *rax = sy_thread_get_rax();
                if (rax == ERROR)
                {
                    goto region_error;
                }

                if (rax)
                {
                    if (for1->key)
                    {
                        sy_record_type_t *type = (sy_record_type_t *)rax->value;
                        sy_node_for_t *for2 = (sy_node_for_t *)type->type->value;
                        if (sy_execute_id_cmp(for1->key, for2->key) == 1)
                        {
                            rax->link -= 1;
                            if (sy_thread_set_rax(NULL) < 0)
                            {
                                goto region_error;
                            }
                            ret_code = 0;

                            goto region_continue_loop;
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
            else
            if (r2 < 0)
            {
                ret_code = r2;
                goto region_continue_loop;
            }
        }

        region_continue_loop:
        if (for1->incrementor)
        {
            for (sy_node_t *item1 = for1->incrementor;item1 != NULL;item1 = item1->next)
            {
                int32_t r1 = sy_execute_assign(item1, strip, applicant, NULL);
                if (r1 == -1)
                {
                    goto region_error;
                }
            }
        }

        goto region_start_loop;
    }

    region_end_loop:
    if (sy_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }

    return ret_code;

    region_error:
    return -1;
}

static int32_t 
sy_execute_try(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_try_t *try1 = (sy_node_try_t *)node->value;

    int32_t r2 = sy_execute_body(try1->body, strip, applicant);
    if (r2 == -4)
    {
        sy_record_t *rax = sy_thread_get_rax();
        if (rax == ERROR)
        {
            return -1;
        }

        if (try1->catchs)
        {
            sy_node_catch_t *catch1 = (sy_node_catch_t *)try1->catchs->value;
            int32_t r1 = sy_execute_parameters_check_by_one_argument(strip, catch1->parameters, rax, applicant);
            if (r1 == -1)
            {
                return -1;
            }
            else
            if (r1 == 0)
            {
                return r2;
            }

            if (sy_execute_parameters_substitute_by_one_argument(try1->catchs, try1->catchs, strip, catch1->parameters, rax, applicant) < 0)
            {
                return -1;
            }

            rax->link -= 1;
            if (sy_thread_set_rax(NULL) < 0)
            {
                return -1;
            }

            if (catch1->parameters)
            {
                sy_node_block_t *parameters = (sy_node_block_t *)catch1->parameters->value;
                for (sy_node_t *item = parameters->items;item != NULL;item = item->next)
                {
                    sy_node_parameter_t *parameter = (sy_node_parameter_t *)item->value;
                    sy_entry_t *entry = sy_strip_input_find(strip, try1->catchs, parameter->key);
                    if (entry == ERROR)
                    {
                        return -1;
                    }
                    
                    sy_entry_t *entry2 = sy_strip_variable_push(strip, try1->catchs, item, parameter->key, entry->value);
                    if (entry2 == ERROR)
                    {
                        return -1;
                    }
                }
            }

            int32_t r3 = sy_execute_body(catch1->body, strip, applicant);
            if (r3 < 0)
            {
                return r3;
            }

            if (sy_strip_variable_remove_by_scope(strip, try1->catchs) < 0)
            {
                return -1;
            }
        }
    }
    else
    if (r2 < 0)
    {
        return r2;
    }

    return 0;
}

static int32_t 
sy_execute_if(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_if_t *if1 = (sy_node_if_t *)node->value;
    if (if1->condition != NULL)
    {
        sy_record_t *condition = sy_execute_expression(if1->condition, strip, applicant, NULL);
        if (condition == ERROR)
        {            
            return -1;
        }

        int32_t truthy = sy_execute_truthy(condition);
        condition->link -= 1;

        if (!truthy)
        {
            goto region_else;
        }
    }

    if (if1->then_body != NULL)
    {
        int32_t r1 = sy_execute_body(if1->then_body, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }

    return 0;

    region_else:
    if (if1->else_body != NULL)
    {
        sy_node_t *else_body1 = if1->else_body;

        if (else_body1->kind == NODE_KIND_IF)
        {
            int32_t r1 = sy_execute_if(if1->else_body, strip, applicant);
            if (r1 < 0)
            {
                return r1;
            }
        }
        else
        {
            int32_t r1 = sy_execute_body(if1->else_body, strip, applicant);
            if (r1 < 0)
            {
                return r1;
            }
        }
    }

    return 0;
}

static int32_t 
sy_execute_throw(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

    if (unary->right)
    {
        sy_record_t *value = sy_execute_expression(unary->right, strip, applicant, NULL);
        if (value == ERROR)
        {            
            return -1;
        }

        if (sy_thread_set_rax(value) < 0)
        {
            return -1;
        }
    }

    return -4;
}

static int32_t 
sy_execute_break(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

    if (unary->right)
    {
        sy_record_t *value = sy_execute_expression(unary->right, strip, applicant, NULL);
        if (value == ERROR)
        {            
            return -1;
        }

        if (value->kind != RECORD_KIND_TYPE)
        {
            value->link -= 1;
            goto region_error;
        }

        sy_record_type_t *type = (sy_record_type_t *)value->value;
        if (type->type->kind != NODE_KIND_FOR)
        {
            value->link -= 1;
            goto region_error;
        }

        if (sy_thread_set_rax(value) < 0)
        {
            return -1;
        }
    }

    return -2;

    region_error:
    sy_error_type_by_node(unary->right, "for loop is not mentioned");
    return -1;
}

static int32_t 
sy_execute_continue(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

    if (unary->right)
    {
        sy_record_t *value = sy_execute_expression(unary->right, strip, applicant, NULL);
        if (value == ERROR)
        {            
            return -1;
        }

        if (value->kind != RECORD_KIND_TYPE)
        {
            value->link -= 1;
            goto region_error;
        }

        sy_record_type_t *type = (sy_record_type_t *)value->value;
        if (type->type->kind != NODE_KIND_FOR)
        {
            value->link -= 1;
            goto region_error;
        }

        if (sy_thread_set_rax(value) < 0)
        {
            return -1;
        }
    }

    return -3;

    region_error:
    sy_error_type_by_node(unary->right, "for loop is not mentioned");
    return -1;
}

static int32_t 
sy_execute_return(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

    if (unary->right)
    {
        sy_record_t *value = sy_execute_expression(unary->right, strip, applicant, NULL);
        if (value == ERROR)
        {            
            return -1;
        }

        if (sy_thread_set_rax(value) < 0)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t 
sy_execute_statement(sy_node_t *scope, sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t r1 = sy_execute_var(scope, node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else
    if (node->kind == NODE_KIND_FOR)
    {
        int32_t r1 = sy_execute_for(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else
    if (node->kind == NODE_KIND_IF)
    {
        int32_t r1 = sy_execute_if(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else
    if (node->kind == NODE_KIND_TRY)
    {
        int32_t r1 = sy_execute_try(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else
    if (node->kind == NODE_KIND_BREAK)
    {
        int32_t r1 = sy_execute_break(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else
    if (node->kind == NODE_KIND_CONTINUE)
    {
        int32_t r1 = sy_execute_continue(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else
    if (node->kind == NODE_KIND_RETURN)
    {
        int32_t r1 = sy_execute_return(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else
    if (node->kind == NODE_KIND_THROW)
    {
        int32_t r1 = sy_execute_throw(node, strip, applicant);
        if (r1 < 0)
        {
            return r1;
        }
    }
    else
    {
        int32_t r1 = sy_execute_assign(node, strip, applicant, NULL);
        if (r1 < 0)
        {
            return r1;
        }
    }

    return 0;
}

static int32_t 
sy_execute_body(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        int32_t r1 = sy_execute_statement(node, item, strip, applicant);
        if (r1 == -1)
        {
            if (sy_strip_variable_remove_by_scope(strip, node) < 0)
            {
                return -1;
            }
            
            return -1;
        }
        else
        if (r1 < 0)
        {
            if (sy_strip_variable_remove_by_scope(strip, node) < 0)
            {
                return -1;
            }
            return r1;
        }
    }

    if (sy_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }

    return 0;
}

int32_t 
sy_execute_run_fun(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_fun_t *fun = (sy_node_fun_t *)node->value;

    if (fun->parameters)
    {
        sy_node_block_t *parameters = (sy_node_block_t *)fun->parameters->value;
        for (sy_node_t *item = parameters->items;item != NULL;item = item->next)
        {
            sy_node_parameter_t *parameter = (sy_node_parameter_t *)item->value;
            sy_entry_t *entry = sy_strip_input_find(strip, node, parameter->key);
            if (entry == ERROR)
            {
                return -1;
            }
            
            sy_entry_t *entry2 = sy_strip_variable_push(strip, node, item, parameter->key, entry->value);
            if (entry2 == ERROR)
            {
                return -1;
            }
        }
    }
    
    int32_t r1 = sy_execute_body(fun->body, strip, applicant);
    
    if (sy_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }

    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 < 0)
    {
        if (r1 == -4)
        {
            return r1;
        }
        sy_error_runtime_by_node(node, "there is no loop");
        return -1;
    }

    return 0;
}

int32_t 
sy_execute_run_lambda(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant)
{
    sy_node_lambda_t *fun = (sy_node_lambda_t *)node->value;
    
    if (fun->parameters)
    {
        sy_node_block_t *parameters = (sy_node_block_t *)fun->parameters->value;
        for (sy_node_t *item = parameters->items;item != NULL;item = item->next)
        {
            sy_node_parameter_t *parameter = (sy_node_parameter_t *)item->value;
            sy_entry_t *entry = sy_strip_input_find(strip, node, parameter->key);
            if (entry == ERROR)
            {
                return -1;
            }

            sy_entry_t *entry2 = sy_strip_variable_push(strip, node, item, parameter->key, entry->value);
            if (entry2 == ERROR)
            {
                return -1;
            }
        }
    }
    
    int32_t r1 = sy_execute_body(fun->body, strip, applicant);
    
    if (sy_strip_variable_remove_by_scope(strip, node) < 0)
    {
        return -1;
    }

    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 < 0)
    {
        if (r1 == -4)
        {
            return r1;
        }
        sy_error_runtime_by_node(node, "there is no loop");
        return -1;
    }

    return 0;
}

static int32_t 
sy_execute_module(sy_node_t *node)
{
    sy_strip_t *strip = sy_strip_create(NULL);
    if (strip == ERROR)
    {
        return -1;
    }

    sy_node_block_t *block = (sy_node_block_t *)node->value;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        if (item->kind == NODE_KIND_CLASS)
        {
			continue;
        }
        else
        if (item->kind == NODE_KIND_USING)
        {
            continue;
        }
        else
        {
            int32_t r = sy_execute_statement(node, item, strip, node);
            if (r < 0)
            {
                if (r == -4)
                {
                    sy_error_runtime_by_node(node, "expection is not handled");
                }
                return -1;
            }
        }
    }

    if (sy_strip_destroy(strip) < 0)
    {
        return -1;
    }

    return 0;
}

int32_t 
sy_execute_run(sy_node_t *root)
{
    if (sy_execute_module(root)  < 0)
    {
        return -1;
    }
    return 0;
}
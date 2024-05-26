#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../parser/syntax/syntax.h"
#include "../error.h"
#include "../mutex.h"
#include "record.h"
#include "garbage.h"
#include "symbol_table.h"
#include "strip.h"

static int32_t
sy_execute_id_cmp(sy_node_t *n1, sy_node_t *n2)
{
	sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;
	sy_node_basic_t *nb2 = (sy_node_basic_t *)n2->value;

	return (strcmp(nb1->value, nb2->value) == 0);
}

sy_record_t *
sy_execute_expression(sy_node_t *node, strip_t *strip)
{
    return NULL;
}

int32_t
sy_execute_type_check_by_type(sy_record_t *record_type1, sy_record_t *record_type2, strip_t *strip)
{
    sy_record_type_t *type1 = (sy_record_type_t *)record_type1->value;
	sy_record_type_t *type2 = (sy_record_type_t *)record_type2->value;

	if (type1->type->kind == NODE_KIND_KINT8)
	{
		if (type2->type->kind == NODE_KIND_KINT8)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KINT16)
	{
		if (type2->type->kind == NODE_KIND_KINT16)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KINT32)
	{
		if (type2->type->kind == NODE_KIND_KINT32)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KINT64)
	{
		if (type2->type->kind == NODE_KIND_KINT64)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KUINT8)
	{
		if (type2->type->kind == NODE_KIND_KUINT8)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KUINT16)
	{
		if (type2->type->kind == NODE_KIND_KUINT16)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KUINT32)
	{
		if (type2->type->kind == NODE_KIND_KUINT32)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KUINT64)
	{
		if (type2->type->kind == NODE_KIND_KUINT64)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KBIGINT)
	{
		if (type2->type->kind == NODE_KIND_KBIGINT)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KFLOAT32)
	{
		if (type2->type->kind == NODE_KIND_KFLOAT32)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KFLOAT64)
	{
		if (type2->type->kind == NODE_KIND_KFLOAT64)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KBIGFLOAT)
	{
		if (type2->type->kind == NODE_KIND_KBIGFLOAT)
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
		sy_node_block_t *block1 = (sy_node_block_t *)type1->type->value;
		if (type2->type->kind == NODE_KIND_OBJECT)
		{
			uint64_t cnt1 = 0;
			sy_node_block_t *block2 = (sy_node_block_t *)type1->type->value;
			for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
			{
				cnt1 += 1;

				sy_node_pair_t *pair1 = (sy_node_pair_t *)item1->value;
				for (sy_node_t *item2 = block2->items;item2 != NULL;item2 = item2->next)
				{
					sy_node_pair_t *pair2 = (sy_node_pair_t *)item2->value;
					if (sy_execute_id_cmp(pair1->key, pair2->key) == 1)
					{
						sy_record_t *pair_value1 = sy_execute_expression(pair1->value, strip);
						if (pair_value1 == ERROR)
						{
							return -1;
						}
                        else
                        if (pair_value1 == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)pair1->key;
                            sy_error_semantic_by_node(pair1->key, "type of '%s' is not defined", basic1->value);
                            return -1;
                        }

						if (pair_value1->kind != RECORD_KIND_TYPE)
						{
							sy_node_basic_t *basic1 = (sy_node_basic_t *)pair1->key;
                            sy_error_semantic_by_node(pair1->key, "type of '%s' is not a type", basic1->value);
                            if (pair_value1->reference == 0)
                            {
                                if (sy_record_destroy(pair_value1) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
						}

						sy_record_t *pair_value2 = sy_execute_expression(pair2->value, strip);
						if (pair_value2 == ERROR)
						{
							if (pair_value1->reference == 0)
                            {
                                if (sy_record_destroy(pair_value1) < 0)
                                {
                                    return -1;
                                }
                            }
							return -1;
						}
                        else
                        if (pair_value2 == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)pair2->key;
                            sy_error_semantic_by_node(pair2->key, "type of '%s' is not defined", basic1->value);
                            return -1;
                        }

						if (pair_value2->kind != RECORD_KIND_TYPE)
						{
							sy_node_basic_t *basic1 = (sy_node_basic_t *)pair2->key;
							sy_error_semantic_by_node(pair2->key, "type of '%s' is not a type", basic1->value);
                            if (pair_value1->reference == 0)
                            {
                                if (sy_record_destroy(pair_value1) < 0)
                                {
                                    if (pair_value2->reference == 0)
                                    {
                                        if (sy_record_destroy(pair_value2) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    return -1;
                                }
                            }
                            if (pair_value2->reference == 0)
                            {
                                if (sy_record_destroy(pair_value2) < 0)
                                {
                                    return -1;
                                }
                            }
							return -1;
						}

						int32_t r1 = sy_execute_type_check_by_type(pair_value1, pair_value2, strip);
                        if (r1 == -1)
						{
                            if (pair_value1->reference == 0)
                            {
                                if (sy_record_destroy(pair_value1) < 0)
                                {
                                    if (pair_value2->reference == 0)
                                    {
                                        if (sy_record_destroy(pair_value2) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    return -1;
                                }
                            }
                            if (pair_value2->reference == 0)
                            {
                                if (sy_record_destroy(pair_value2) < 0)
                                {
                                    return -1;
                                }
                            }
							return -1;
						}
                        else
						if (r1 == 0)
						{
							if (pair_value1->reference == 0)
                            {
                                if (sy_record_destroy(pair_value1) < 0)
                                {
                                    if (pair_value2->reference == 0)
                                    {
                                        if (sy_record_destroy(pair_value2) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    return -1;
                                }
                            }
                            if (pair_value2->reference == 0)
                            {
                                if (sy_record_destroy(pair_value2) < 0)
                                {
                                    return -1;
                                }
                            }

							return 0;
						}

						if (pair_value1->reference == 0)
                        {
                            if (sy_record_destroy(pair_value1) < 0)
                            {
                                if (pair_value2->reference == 0)
                                {
                                    if (sy_record_destroy(pair_value2) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                return -1;
                            }
                        }
                        if (pair_value2->reference == 0)
                        {
                            if (sy_record_destroy(pair_value2) < 0)
                            {
                                return -1;
                            }
                        }
					}
				}
			}

			uint64_t cnt2 = 0;
			for (sy_node_t *item2 = block2->items;item2 != NULL;item2 = item2->next)
			{
				cnt2 += 1;
			}

			if (cnt1 != cnt2)
			{
				return 0;
			}

			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_ARRAY)
	{
		sy_node_carrier_t *carrier1 = (sy_node_carrier_t *)type1->type->value;
		if (type2->type->kind == NODE_KIND_ARRAY)
		{
			sy_node_carrier_t *carrier2 = (sy_node_carrier_t *)type2->type->value;

			sy_record_t *array_base1 = sy_execute_expression(carrier1->base, strip);
			if (array_base1 == ERROR)
			{
				return -1;
			}
            else
            if (array_base1 == NULL)
            {
                sy_error_semantic_by_node(carrier1->base, "type of '%s' is not defined", "array");
                return -1;
            }

			if (array_base1->kind != RECORD_KIND_TYPE)
			{
				sy_error_semantic_by_node(carrier1->base, "base of '%s' is not a type", "array");
                if (array_base1->reference == 0)
                {
                    if (sy_record_destroy(array_base1) < 0)
                    {
                        return -1;
                    }
                }
				return -1;
			}

			sy_record_t *array_base2 = sy_execute_expression(carrier2->base, strip);
			if (array_base2 == ERROR)
			{
				if (array_base1->reference == 0)
                {
                    if (sy_record_destroy(array_base1) < 0)
                    {
                        return -1;
                    }
                }
				return -1;
			}
            else
            if (array_base2 == NULL)
            {
                sy_error_semantic_by_node(carrier2->base, "type of '%s' is not defined", "array");
                return -1;
            }

			if (array_base2->kind != RECORD_KIND_TYPE)
			{
				sy_error_semantic_by_node(carrier2->base, "base of '%s' is not a type", "array");
                if (array_base1->reference == 0)
                {
                    if (sy_record_destroy(array_base1) < 0)
                    {
                        if (array_base2->reference == 0)
                        {
                            if (sy_record_destroy(array_base2) < 0)
                            {
                                return -1;
                            }
                        }
                        return -1;
                    }
                }
                if (array_base2->reference == 0)
                {
                    if (sy_record_destroy(array_base2) < 0)
                    {
                        return -1;
                    }
                }
				return -1;
			}

			int32_t r1 = sy_execute_type_check_by_type(array_base1, array_base2, strip);
            if (r1 == -1)
			{
                if (array_base1->reference == 0)
                {
                    if (sy_record_destroy(array_base1) < 0)
                    {
                        if (array_base2->reference == 0)
                        {
                            if (sy_record_destroy(array_base2) < 0)
                            {
                                return -1;
                            }
                        }
                        return -1;
                    }
                }
                if (array_base2->reference == 0)
                {
                    if (sy_record_destroy(array_base2) < 0)
                    {
                        return -1;
                    }
                }
				return -1;
			}
            else
			if (r1 == 0)
			{
				if (array_base1->reference == 0)
                {
                    if (sy_record_destroy(array_base1) < 0)
                    {
                        if (array_base2->reference == 0)
                        {
                            if (sy_record_destroy(array_base2) < 0)
                            {
                                return -1;
                            }
                        }
                        return -1;
                    }
                }
                if (array_base2->reference == 0)
                {
                    if (sy_record_destroy(array_base2) < 0)
                    {
                        return -1;
                    }
                }
				return 0;
			}

			if (array_base1->reference == 0)
            {
                if (sy_record_destroy(array_base1) < 0)
                {
                    if (array_base2->reference == 0)
                    {
                        if (sy_record_destroy(array_base2) < 0)
                        {
                            return -1;
                        }
                    }
                    return -1;
                }
            }
            if (array_base2->reference == 0)
            {
                if (sy_record_destroy(array_base2) < 0)
                {
                    return -1;
                }
            }

			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_TUPLE)
	{
		sy_node_block_t *block1 = (sy_node_block_t *)type1->type->value;
		if (type2->type->kind == NODE_KIND_OBJECT)
		{
			sy_node_block_t *block2 = (sy_node_block_t *)type1->type->value;
			sy_node_t *item2 = block2->items;
			for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next, item2 = item2->next)
			{
				if (item2 == NULL)
				{
					return 0;
				}

				sy_record_t *tuple_item1 = sy_execute_expression(item1, strip);
				if (tuple_item1 == ERROR)
				{
					return -1;
				}
                else
                if (tuple_item1 == NULL)
				{
                    sy_error_semantic_by_node(item1, "type of '%s' is not defined", "tuple item");
                    return -1;
                }

				if (tuple_item1->kind != RECORD_KIND_TYPE)
				{
                    sy_error_semantic_by_node(item1, "type of '%s' is not a type", "tuple item");
					if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            return -1;
                        }
                    }
					return -1;
				}

				sy_record_t *tuple_item2 = sy_execute_expression(item2, strip);
				if (tuple_item2 == ERROR)
				{
                    if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            if (tuple_item2->reference == 0)
                            {
                                if (sy_record_destroy(tuple_item2) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }
                    }
                    if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            return -1;
                        }
                    }
					return -1;
				}
                else
                if (tuple_item2 == NULL)
				{
                    sy_error_semantic_by_node(item2, "type of '%s' is not defined", "tuple item");
                    if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            if (tuple_item2->reference == 0)
                            {
                                if (sy_record_destroy(tuple_item2) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }
                    }
                    if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            return -1;
                        }
                    }
					return -1;
				}

				if (tuple_item2->kind != RECORD_KIND_TYPE)
				{
                    sy_error_semantic_by_node(item2, "type of '%s' is not a type", "tuple item");
                    if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            if (tuple_item2->reference == 0)
                            {
                                if (sy_record_destroy(tuple_item2) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }
                    }
                    if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            return -1;
                        }
                    }
					return -1;
				}

				int32_t r1 = sy_execute_type_check_by_type(tuple_item1, tuple_item2, strip);
                if (r1 == -1)
				{
                    if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            if (tuple_item2->reference == 0)
                            {
                                if (sy_record_destroy(tuple_item2) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }
                    }
                    if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            return -1;
                        }
                    }
					return -1;
				}
                else
				if (r1 == 0)
				{
					if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            if (tuple_item2->reference == 0)
                            {
                                if (sy_record_destroy(tuple_item2) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }
                    }
                    if (tuple_item1->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item1) < 0)
                        {
                            return -1;
                        }
                    }
					return 0;
				}

				if (tuple_item1->reference == 0)
                {
                    if (sy_record_destroy(tuple_item1) < 0)
                    {
                        if (tuple_item2->reference == 0)
                        {
                            if (sy_record_destroy(tuple_item2) < 0)
                            {
                                return -1;
                            }
                        }
                        return -1;
                    }
                }
                if (tuple_item1->reference == 0)
                {
                    if (sy_record_destroy(tuple_item1) < 0)
                    {
                        return -1;
                    }
                }
			}
		
			if (item2 != NULL)
			{
				return 0;
			}

			return 1;
		}
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
			sy_node_block_t *block1 = (sy_node_block_t *)class1->generics;

			for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
			{
				sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;

				sy_strip_entry_t *strip_entry1 = sy_strip_variable_find(type1->value, generic1->key);
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

				sy_strip_entry_t *strip_entry2 = sy_strip_variable_find(type2->value, generic1->key);
                if (strip_entry2 == ERROR)
				{
                    return -1;
                }
				if (strip_entry2 == NULL)
				{
					sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
					sy_error_runtime_by_node(generic1->key, "Runtime:'%s' is not initialized", basic1->value);
					return -1;
				}

				int32_t r1 = sy_execute_type_check_by_type(strip_entry1->value, strip_entry2->value, strip);
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
		else
		{
			return 0;
		}
	}

	return 0;
}

int32_t
sy_execute_value_check_by_type(sy_record_t *value, sy_record_t *type, strip_t *strip)
{
    if (!value)
    {
        return 0;
    }

    sy_record_type_t *type1 = (sy_record_type_t *)type->value;

	if (type1->type->kind == NODE_KIND_KINT8)
	{
		if (value->kind == RECORD_KIND_INT8)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KINT16)
	{
		if (value->kind == RECORD_KIND_INT16)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KINT32)
	{
		if (value->kind == RECORD_KIND_INT32)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KINT64)
	{
		if (value->kind == RECORD_KIND_INT64)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KUINT8)
	{
		if (value->kind == RECORD_KIND_UINT8)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KUINT16)
	{
		if (value->kind == RECORD_KIND_UINT16)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KUINT32)
	{
		if (value->kind == RECORD_KIND_UINT32)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KUINT64)
	{
		if (value->kind == RECORD_KIND_UINT64)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KBIGINT)
	{
		if (value->kind == RECORD_KIND_BIGINT)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KFLOAT32)
	{
		if (value->kind == RECORD_KIND_FLOAT32)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KFLOAT64)
	{
		if (value->kind == RECORD_KIND_FLOAT64)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	if (type1->type->kind == NODE_KIND_KBIGFLOAT)
	{
		if (value->kind == RECORD_KIND_BIGFLOAT)
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
		sy_node_block_t *block1 = (sy_node_block_t *)type1->type->value;
		if (value->kind == RECORD_KIND_OBJECT)
		{
			sy_record_object_t *object1 = (sy_record_object_t *)value->value;
			if (block1->items != NULL)
			{
				if (object1 == NULL)
				{
					return 0;
				}

				uint64_t cnt1 = 0;
				while (object1 != NULL)
				{
					int32_t found = 0;
					for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
					{
						sy_node_pair_t *pair1 = (sy_node_pair_t *)item1->value;
						if (sy_execute_id_cmp(pair1->key, object1->key) == 1)
						{
							sy_record_t *pair_value = sy_execute_expression(pair1->value, strip);
							if (pair_value == ERROR)
							{
								return -1;
							}
                            else
                            if (pair_value == NULL)
							{
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)pair1->key;
                                sy_error_semantic_by_node(pair1->key, "type of '%s' is not defined", basic1->value);
                                return -1;
							}

							if (pair_value->kind != RECORD_KIND_TYPE)
							{
								sy_node_basic_t *basic1 = (sy_node_basic_t *)pair1->key;
								sy_error_semantic_by_node(pair1->key, "type of '%s' is not a type", basic1->value);
                                if (pair_value->reference == 0)
                                {
                                    if (sy_record_destroy(pair_value) < 0)
                                    {
                                        return -1;
                                    }
                                }
								return -1;
							}

							int32_t r1 = sy_execute_value_check_by_type(object1->value, pair_value, strip);
                            if (r1 == -1)
							{
                                if (pair_value->reference == 0)
                                {
                                    if (sy_record_destroy(pair_value) < 0)
                                    {
                                        return -1;
                                    }
                                }
								return -1;
							}
                            else
							if (r1 == 0)
							{
								if (pair_value->reference == 0)
                                {
                                    if (sy_record_destroy(pair_value) < 0)
                                    {
                                        return -1;
                                    }
                                }
								return 0;
							}

							if (pair_value->reference == 0)
                            {
                                if (sy_record_destroy(pair_value) < 0)
                                {
                                    return -1;
                                }
                            }

							object1 = object1->next;
							found = 1;
							break;
						}
					}

					if (found == 0)
					{
						return 0;
					}

					cnt1 += 1;
				}

				uint64_t cnt2 = 0;
				for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
				{
					cnt2 += 1;
				}

				if (cnt1 != cnt2)
				{
					return 0;
				}

				return 1;
			}
			else
			{
				if (object1->value == NULL)
				{
					return 1;
				}
				return 0;
			}
		}
		return 0;
	}
	else
	if (type1->type->kind == NODE_KIND_ARRAY)
	{
		sy_node_carrier_t *carrier1 = (sy_node_carrier_t *)type1->type->value;
		if (value->kind == RECORD_KIND_TUPLE)
		{
			sy_record_t *tuple_item = sy_execute_expression(carrier1->base, strip);
			if (tuple_item == ERROR)
			{
				return -1;
			}
            else
            if (tuple_item == NULL)
            {
                sy_error_semantic_by_node(carrier1->base, "base of '%s' is not defined", "array");
                return -1;
            }

			if (tuple_item->kind != RECORD_KIND_TYPE)
			{
                sy_error_semantic_by_node(carrier1->base, "type of '%s' is not a type", "tuple item");
                if (tuple_item->reference == 0)
                {
                    if (sy_record_destroy(tuple_item) < 0)
                    {
                        return -1;
                    }
                }
                return -1;
			}

			sy_record_tuple_t *tuple1 = (sy_record_tuple_t *)value->value;
			while (tuple1 != NULL)
			{
				int32_t r1 = sy_execute_value_check_by_type(tuple1->value, tuple_item, strip);
                if (r1 == -1)
				{
                    if (tuple_item->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item) < 0)
                        {
                            return -1;
                        }
                    }
					return -1;
				}
                else
				if (r1 == 0)
				{
					if (tuple_item->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item) < 0)
                        {
                            return -1;
                        }
                    }
					return 0;
				}

				if (tuple_item->reference == 0)
                {
                    if (sy_record_destroy(tuple_item) < 0)
                    {
                        return -1;
                    }
                }

				tuple1 = tuple1->next;
			}

			return 1;
		}
		return 0;
	}
	else
	if (type1->type->kind == NODE_KIND_TUPLE)
	{
		sy_node_block_t *block1 = (sy_node_block_t *)type1->type->value;
		if (value->kind == RECORD_KIND_TUPLE)
		{
			sy_record_tuple_t *tuple1 = (sy_record_tuple_t *)value->value;
			if (block1->items != NULL)
			{
				if (tuple1 == NULL)
				{
					return 0;
				}

				for (sy_node_t *item1 = block1->items; (item1 != NULL);item1 = item1->next, tuple1 = tuple1->next)
				{
					if (tuple1 == NULL)
					{
						return 0;
					}

					sy_record_t *tuple_item = sy_execute_expression(item1, strip);
					if (tuple_item == ERROR)
					{
						return -1;
					}
                    else
                    if (tuple_item == NULL)
                    {
                        sy_error_semantic_by_node(item1, "type of '%s' is not defined", "tuple item");
                        return -1;
                    }

					if (tuple_item->kind != RECORD_KIND_TYPE)
					{
						sy_error_semantic_by_node(item1, "type of '%s' is not a type", "tuple item");
                        if (tuple_item->reference == 0)
                        {
                            if (sy_record_destroy(tuple_item) < 0)
                            {
                                return -1;
                            }
                        }
						return -1;
					}

					int32_t r1 = sy_execute_value_check_by_type(tuple1->value, tuple_item, strip);
                    if (r1 == -1)
					{
                        if (tuple_item->reference == 0)
                        {
                            if (sy_record_destroy(tuple_item) < 0)
                            {
                                return -1;
                            }
                        }
						return -1;
					}
                    else
					if (r1 == 0)
					{
						if (tuple_item->reference == 0)
                        {
                            if (sy_record_destroy(tuple_item) < 0)
                            {
                                return -1;
                            }
                        }
						return 0;
					}

					if (tuple_item->reference == 0)
                    {
                        if (sy_record_destroy(tuple_item) < 0)
                        {
                            return -1;
                        }
                    }
				}

				if (tuple1 != NULL)
				{
					return 0;
				}

				return 1;
			}
			else
			{
				if (tuple1->value == NULL)
				{
					return 1;
				}
				return 0;
			}
		}
		return 0;
	}
	else
	if (type1->type->kind == NODE_KIND_CLASS)
	{
		if (value->kind == RECORD_KIND_STRUCT)
		{
			sy_record_struct_t *struct1 = (sy_record_struct_t *)value->value;
			if (type1->type->id != struct1->type->id)
			{
				return 0;
			}

			sy_node_class_t *class1 = (sy_node_class_t *)struct1->type;
			sy_node_block_t *block1 = (sy_node_block_t *)class1->generics;
			if (block1 != NULL)
			{
				for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
				{
					sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;

					sy_strip_entry_t *strip_entry1 = sy_strip_variable_find(struct1->value, generic1->key);
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

					sy_strip_entry_t *strip_entry2 = sy_strip_variable_find(type1->value, generic1->key);
                    if (strip_entry2 == ERROR)
					{
                        return -1;
                    }
					if (strip_entry2 == NULL)
					{
						sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
						sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
						return -1;
					}

					int32_t r1 = sy_execute_type_check_by_type(strip_entry1->value, strip_entry2->value, strip);
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
			}
			return 1;
		}
		return 0;
	}
    return 0;
}

sy_record_t *
sy_execute_value_convert_by_type(sy_record_t *value, sy_record_t *type, strip_t *strip)
{
    return value;
}

sy_record_t *
sy_execute_attribute_from_struct(sy_record_t *record, sy_node_t *key)
{
    assert (record->kind == RECORD_KIND_STRUCT);

    sy_record_struct_t *struct1 = (sy_record_struct_t *)record->value;
    sy_node_t *type = struct1->type;
    sy_node_class_t *class1 = (sy_node_class_t *)type->value;

	for (sy_node_t *item1 = class1->block; item1 != NULL; item1 = item1->next)
	{
		if (item1->kind == NODE_KIND_PROPERTY)
		{
			sy_node_property_t *property1 = (sy_node_property_t *)item1->value;
			if ((property1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
			{
				if (sy_execute_id_cmp(property1->key, key) == 1)
				{
                    sy_strip_entry_t *entry = sy_strip_variable_find(struct1->value, property1->key);
                    if (entry == ERROR)
                    {
                        return ERROR;
                    }
                    else
                    if (entry == NULL)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)property1->key;
                        sy_error_runtime_by_node(property1->key, "'%s' is not initialized", basic1->value);
                        return ERROR;
                    }

                    return entry->value;
                }
            }
        }
    }

    sy_node_block_t *block1 = (sy_node_block_t *)class1->heritages->value;
	for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
	{
		sy_node_heritage_t *heritage1 = (sy_node_heritage_t *)item1->value;
        sy_strip_entry_t *entry = sy_strip_variable_find(struct1->value, heritage1->key);
        if (entry == ERROR)
        {
            return ERROR;
        }
        else
        if (entry == NULL)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage1->key;
            sy_error_runtime_by_node(heritage1->key, "'%s' is not initialized", basic1->value);
            return ERROR;
        }

        sy_record_t *r1 = sy_execute_attribute_from_struct(entry->value, key);
        if (r1 == ERROR)
        {
            return ERROR;
        }
        else
        if (r1 != NULL)
        {
            return r1;
        }
    }

    return NULL;
}

sy_record_t *
sy_execute_attribute_from_type(sy_record_t *record, sy_node_t *key)
{
    assert (record->kind == RECORD_KIND_TYPE);

    sy_record_type_t *type1 = (sy_record_type_t *)record->value;
    sy_node_t *node = type1->type;

    if (node->kind == NODE_KIND_CLASS)
    {
        sy_node_class_t *class1 = (sy_node_class_t *)node->value;
        for (sy_node_t *item1 = class1->block; item1 != NULL; item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                sy_node_property_t *property1 = (sy_node_property_t *)item1->value;
                if ((property1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                {
                    if ((property1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                    {
                        if (sy_execute_id_cmp(property1->key, key) == 1)
                        {
                            sy_symbol_table_entry_t *entry = sy_symbol_table_find(node, property1->key);
                            if (entry == ERROR)
                            {
                                return ERROR;
                            }
                            else
                            if (entry == NULL)
                            {
                                sy_record_t *record_value = sy_execute_expression(property1->value, type1->value);
                                if (record_value == ERROR)
                                {
                                    return ERROR;
                                }

                                if (record_value)
                                {
                                    if (record_value->reference > 0)
                                    {
                                        if ((property1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                                        {
                                            record_value = sy_record_copy(record_value);
                                            if (record_value == ERROR)
                                            {
                                                return ERROR;
                                            }
                                            record_value->reference += 1;
                                        }
                                    }
                                    else
                                    {
                                        if ((property1->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                        {
                                            sy_error_semantic_by_node(property1->key, "the unreferenced type is assigned to the reference type");
                                            return ERROR;
                                        }
                                        else
                                        {
                                            record_value->reference += 1;
                                        }
                                    }
                                }

                                if (ERROR == sy_symbol_table_push(node, item1, property1->key, record_value))
                                {
                                    if (record_value)
                                    {
                                        if ((property1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                                        {
                                            record_value->reference = 0;
                                            if (sy_record_destroy(record_value) < 0)
                                            {
                                                return ERROR;
                                            }
                                        }
                                    }
                                    return ERROR;
                                }

                                return record_value;
                            }
                            return entry->value;
                        }
                    }
                }
            }
        }

        sy_node_block_t *block1 = (sy_node_block_t *)class1->heritages->value;
        for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
        {
            sy_node_heritage_t *heritage1 = (sy_node_heritage_t *)item1->value;

            sy_record_t *record_type = sy_execute_expression(heritage1->type, type1->value);
            if (record_type == ERROR)
            {
                return ERROR;
            }
            else
            if (record_type == NULL)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage1->key;
                sy_error_semantic_by_node(heritage1->key, "type of '%s' is not defined", basic1->value);
                return ERROR;
            }

            if (record_type->kind != RECORD_KIND_TYPE)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage1->key;
                sy_error_semantic_by_node(heritage1->key, "type of '%s' is not a type", basic1->value);
                if (sy_record_destroy(record_type) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }
            else
            {
                sy_record_type_t *type2 = (sy_record_type_t *)record_type->value;
                if (type2->type->kind != NODE_KIND_CLASS)
                {
                    if (type2->type->kind != NODE_KIND_GENERIC)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage1->key;
                        sy_error_semantic_by_node(heritage1->key, "type of '%s' is not an inheritable type", basic1->value);
                        if (sy_record_destroy(record_type) < 0)
                        {
                            return ERROR;
                        }
                        return ERROR;
                    }
                }
            }

            sy_record_t *r1 = sy_execute_attribute_from_type(record_type, key);
            if (r1 == ERROR)
            {
                if (sy_record_destroy(record_type) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }
            else
            if (r1 != NULL)
            {
                if (sy_record_destroy(record_type) < 0)
                {
                    return ERROR;
                }
                return r1;
            }

            if (sy_record_destroy(record_type) < 0)
            {
                return ERROR;
            }
        }
    }

    return NULL;
}

int32_t 
sy_execute_entity(sy_node_t *scope, sy_node_t *node, sy_record_t *value, strip_t *strip)
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
            value_select = sy_execute_attribute_from_struct(value, key_search);
            if (value_select == ERROR)
            {
                return -1;
            }
        }
        else
        if (value->kind == RECORD_KIND_TYPE)
        {
            value_select = sy_execute_attribute_from_type(value, key_search);
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
            value_select = sy_execute_expression(entity->value, strip);
            if (value_select == ERROR)
            {
                return -1;
            }
        }
    } 

    if (value_select)
    {
        if (value_select->reference > 0)
        {
            if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
            {
                value_select = sy_record_copy(value_select);
                if (value_select == ERROR)
                {
                    return -1;
                }
                value_select->reference += 1;
            }
        }
        else
        {
            if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
            {
                sy_error_semantic_by_node(entity->key, "the unreferenced type is assigned to the reference type");
                return -1;
            }
            else
            {
                value_select->reference += 1;
            }
        }
    }

    if ((entity->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
    {
        if (NULL == sy_symbol_table_push(scope, node, entity->key, value_select))
        {
            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_destroy(value_select) < 0)
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
        if (NULL == sy_strip_variable_push(strip, scope, node, entity->key, value_select))
        {
            if (value_select)
            {
                if ((entity->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_destroy(value_select) < 0)
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

int32_t 
sy_execute_set(sy_node_t *scope, sy_node_t *node, sy_node_t *value, strip_t *strip)
{
    sy_record_t *record_value = sy_execute_expression(value, strip);
    if (record_value == ERROR)
    {
        return -1;
    }

    sy_node_block_t *block = (sy_node_block_t *)node->value;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        if (sy_execute_entity(scope, item, record_value, strip) < 0)
        {
            if (ERROR == sy_garbage_push(record_value))
            {
                return -1;
            }
            return -1;
        }
    }

    if (ERROR == sy_garbage_push(record_value))
    {
        return -1;
    }

    return 0;
}

int32_t 
sy_execute_var(sy_node_t *scope, sy_node_t *node, strip_t *strip)
{
    sy_node_var_t *var1 = (sy_node_var_t *)node->value;
    if (var1->key->kind == NODE_KIND_SET)
	{
		if (sy_execute_set(scope, var1->key, var1->value, strip) < 0)
		{
			return -1;
		}
	}
    else
    {
        sy_record_t *record_value = NULL;
        if (var1->value)
        {
            record_value = sy_execute_expression(var1->value, strip);
            if (record_value == ERROR)
            {
                return -1;
            }
        }

        if (record_value)
        {
            if (var1->type)
            {
                sy_record_t *record_type = sy_execute_expression(var1->type, strip);
                if (record_type == ERROR)
                {
                    if (record_value->reference == 0)
                    {
                        if (sy_record_destroy(record_value) < 0)
                        {
                            return -1;
                        }
                    }
                    return -1;
                }
                else
                if (record_type == NULL)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key;
                    sy_error_semantic_by_node(var1->key, "type of '%s' is not defined", basic1->value);
                    if (record_value->reference == 0)
                    {
                        if (sy_record_destroy(record_value) < 0)
                        {
                            return -1;
                        }
                    }
                    return -1;
                }

                if (record_type->kind != RECORD_KIND_TYPE)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key;
                    sy_error_semantic_by_node(var1->key, "type of '%s' is not a type", basic1->value);
                    if (sy_record_destroy(record_type) < 0)
                    {
                        return -1;
                    }
                    if (record_value->reference == 0)
                    {
                        if (sy_record_destroy(record_value) < 0)
                        {
                            return -1;
                        }
                    }
                    return -1;
                }

                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                {
                    int32_t r1 = sy_execute_value_check_by_type(record_value, record_type, strip);
                    if (r1 < 0)
                    {
                        if (sy_record_destroy(record_type) < 0)
                        {
                            return -1;
                        }
                        if (record_value->reference == 0)
                        {
                            if (sy_record_destroy(record_value) < 0)
                            {
                                return -1;
                            }
                        }
                        return -1;
                    }
                    else
                    if (r1 == 0)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key;
                        sy_error_semantic_by_node(var1->key, "value of '%s' is not equal to its type", basic1->value);
                        if (sy_record_destroy(record_type) < 0)
                        {
                            return -1;
                        }
                        if (record_value->reference == 0)
                        {
                            if (sy_record_destroy(record_value) < 0)
                            {
                                return -1;
                            }
                        }
                        return -1;
                    }
                }
                else
                {
                    if (record_value->reference > 0)
                    {
                        record_value = sy_record_copy(record_value);
                    }
                    record_value = sy_execute_value_convert_by_type(record_value, record_type, strip);
                    if (record_value == ERROR)
                    {
                        if (sy_record_destroy(record_type) < 0)
                        {
                            return -1;
                        }
                        if (record_value->reference == 0)
                        {
                            if (sy_record_destroy(record_value) < 0)
                            {
                                return -1;
                            }
                        }
                        return -1;
                    }
                }
            }
        }

        if ((var1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            if (NULL == sy_symbol_table_push(scope, node, var1->key, record_value))
            {
                if (record_value)
                {
                    if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (sy_record_destroy(record_value) < 0)
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
            if (NULL == sy_strip_variable_push(strip, scope, node, var1->key, record_value))
            {
                if (record_value)
                {
                    if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (sy_record_destroy(record_value) < 0)
                        {
                            return -1;
                        }
                    }
                }
                return -1;
            }
        }
    }

    return 0;
}

int32_t 
sy_execute_module(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        if (item->kind == NODE_KIND_VAR)
        {
			if (sy_execute_var(node, item, NULL) < 0)
			{
				return -1;
			}
        }
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
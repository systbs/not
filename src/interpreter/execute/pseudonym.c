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
#include "../entry.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

int32_t
not_execute_type_extends_of_type(not_node_t *node, not_record_t *record_type1, not_record_t *record_type2, not_strip_t *strip, not_node_t *applicant)
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
                not_node_class_t *class1 = (not_node_class_t *)type1->type->value;
                if (class1->heritages)
                {
                    not_node_block_t *block1 = (not_node_block_t *)class1->heritages->value;

                    for (not_node_t *item = block1->items; item != NULL; item = item->next)
                    {
                        not_node_heritage_t *heritage = (not_node_heritage_t *)item->value;

                        not_record_t *record_heritage = not_execute_expression(heritage->type, strip, applicant, NULL);
                        if (record_heritage == ERROR)
                        {
                            return -1;
                        }

                        if (record_heritage->kind != RECORD_KIND_TYPE)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)heritage->key->value;
                            not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                            not_error_type_by_node(heritage->key, "'%s' unexpected type as heritage '%s'",
                                                   basic2->value, basic1->value);

                            if (not_record_link_decrease(record_heritage) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }

                        int32_t r1 = not_execute_type_extends_of_type(node, record_heritage, record_type2, strip, applicant);
                        if (r1 == -1)
                        {
                            if (not_record_link_decrease(record_heritage) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }
                        else if (r1 == 1)
                        {
                            if (not_record_link_decrease(record_heritage) < 0)
                            {
                                return -1;
                            }
                            return 1;
                        }

                        if (not_record_link_decrease(record_heritage) < 0)
                        {
                            return -1;
                        }
                    }
                }
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

                    int32_t r1 = not_execute_type_extends_of_type(node, strip_entry1->value, strip_entry2->value, strip, applicant);
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

not_record_t *
not_execute_pseudonym(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_carrier_t *carrier = (not_node_carrier_t *)node->value;

    not_record_t *base = not_execute_expression(carrier->base, strip, applicant, origin);
    if (base == ERROR)
    {
        return ERROR;
    }

    if (base->kind != RECORD_KIND_TYPE)
    {
        goto region_error_nogen;
    }

    if (base->link > 0)
    {
        base = not_record_copy(base);
        if (base == ERROR)
        {
            return ERROR;
        }
    }

    not_record_type_t *base_type = (not_record_type_t *)base->value;
    not_strip_t *new_strip = NULL;
    not_node_block_t *block1 = NULL;
    if (base_type->type->kind == NODE_KIND_CLASS)
    {
        not_node_class_t *class1 = (not_node_class_t *)base_type->type->value;
        if (!class1->generics)
        {
            goto region_error_nogen;
        }

        new_strip = (not_strip_t *)base_type->value;
        block1 = (not_node_block_t *)class1->generics->value;
    }
    else if (base_type->type->kind == NODE_KIND_FUN)
    {
        not_node_fun_t *fun1 = (not_node_fun_t *)base_type->type->value;
        if (!fun1->generics)
        {
            goto region_error_nogen;
        }

        new_strip = (not_strip_t *)base_type->value;
        block1 = (not_node_block_t *)fun1->generics->value;
    }
    else if (base_type->type->kind == NODE_KIND_LAMBDA)
    {
        not_node_lambda_t *fun1 = (not_node_lambda_t *)base_type->type->value;
        if (!fun1->generics || !fun1->body)
        {
            goto region_error_nogen;
        }

        new_strip = (not_strip_t *)base_type->value;
        block1 = (not_node_block_t *)fun1->generics->value;
    }
    else
    {
        goto region_error_nogen;
    }

    not_node_t *item1 = block1->items;

    not_node_block_t *block2 = carrier->data->value;
    for (not_node_t *item2 = block2->items; item2 != NULL; item2 = item2->next)
    {
        not_node_field_t *field = (not_node_field_t *)item2->value;

        if (!item1)
        {
            if (field->value)
            {
                if (base_type->type->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class1 = (not_node_class_t *)base_type->type->value;
                    not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)field->key->value;
                    not_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", basic1->value, basic2->value);
                    goto region_error;
                }
                else if (base_type->type->kind == NODE_KIND_FUN)
                {
                    not_node_fun_t *fun1 = (not_node_fun_t *)base_type->type->value;
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)field->key->value;
                    not_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", basic1->value, basic2->value);
                    goto region_error;
                }
                else
                {
                    not_node_basic_t *basic2 = (not_node_basic_t *)field->key->value;
                    not_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", "lambda", basic2->value);
                    goto region_error;
                }
            }
            else
            {
                uint64_t cnt1 = 0;
                for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                {
                    cnt1 += 1;
                }

                uint64_t cnt2 = 0;
                for (not_node_t *item1 = block2->items; item1 != NULL; item1 = item1->next)
                {
                    cnt2 += 1;
                }

                if (base_type->type->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class1 = (not_node_class_t *)base_type->type->value;
                    not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(field->key, "'%s' takes %lld positional fields but %lld were given", basic1->value, cnt1, cnt2);
                    goto region_error;
                }
                else if (base_type->type->kind == NODE_KIND_FUN)
                {
                    not_node_fun_t *fun1 = (not_node_fun_t *)base_type->type->value;
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_error_type_by_node(field->key, "'%s' takes %lld positional fields but %lld were given", basic1->value, cnt1, cnt2);
                    goto region_error;
                }
                else
                {
                    not_error_type_by_node(field->key, "'%s' takes %lld positional fields but %lld were given", "lambda", cnt1, cnt2);
                    goto region_error;
                }

                goto region_error;
            }
        }

        if (field->value)
        {
            int8_t found = 0;
            for (not_node_t *item3 = item1; item3 != NULL; item3 = item3->next)
            {
                not_node_generic_t *generic = (not_node_generic_t *)item3->value;
                if (not_execute_id_cmp(field->key, generic->key) == 1)
                {
                    found = 1;
                    not_record_t *record_field = not_execute_expression(field->value, new_strip, applicant, NULL);
                    if (record_field == ERROR)
                    {
                        goto region_error;
                    }

                    if (record_field->kind != RECORD_KIND_TYPE)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)field->key->value;
                        not_error_type_by_node(field->value, "'%s' unsupported type: '%s'",
                                               basic1->value, not_record_type_as_string(record_field));
                        goto region_error;
                    }

                    if (generic->type)
                    {
                        not_record_t *record_gen_type = not_execute_expression(generic->type, new_strip, applicant, NULL);
                        if (record_gen_type == ERROR)
                        {
                            if (not_record_link_decrease(record_field) < 0)
                            {
                                return ERROR;
                            }
                            goto region_error;
                        }

                        if (record_gen_type->kind != RECORD_KIND_TYPE)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)generic->key->value;
                            not_error_type_by_node(generic->type, "'%s' unsupported type: '%s'",
                                                   basic1->value, not_record_type_as_string(record_gen_type));

                            if (not_record_link_decrease(record_field) < 0)
                            {
                                return ERROR;
                            }

                            goto region_error;
                        }

                        int32_t r1 = not_execute_type_extends_of_type(field->key, record_field, record_gen_type, new_strip, applicant);
                        if (r1 < 0)
                        {
                            if (not_record_link_decrease(record_field) < 0)
                            {
                                return ERROR;
                            }
                            if (not_record_link_decrease(record_gen_type) < 0)
                            {
                                return ERROR;
                            }
                            goto region_error;
                        }
                        else if (r1 == 0)
                        {
                            if (not_record_link_decrease(record_field) < 0)
                            {
                                return ERROR;
                            }
                            if (not_record_link_decrease(record_gen_type) < 0)
                            {
                                return ERROR;
                            }
                            not_node_basic_t *basic1 = (not_node_basic_t *)field->key->value;
                            not_error_type_by_node(field->key, "'%s' mismatch: '%s' and '%s'",
                                                   basic1->value, not_record_type_as_string(record_field), not_record_type_as_string(record_gen_type));
                            goto region_error;
                        }

                        if (not_record_link_decrease(record_gen_type) < 0)
                        {
                            return ERROR;
                        }
                    }

                    not_entry_t *entry = not_strip_variable_push(new_strip, base_type->type, item3, generic->key, record_field);
                    if (entry == ERROR)
                    {
                        if (not_record_link_decrease(record_field) < 0)
                        {
                            return ERROR;
                        }
                        goto region_error;
                    }
                    else if (entry == NULL)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)generic->key->value;
                        not_error_type_by_node(field->key, "'%s' already set", basic1->value);

                        if (not_record_link_decrease(record_field) < 0)
                        {
                            return ERROR;
                        }
                        goto region_error;
                    }
                    break;
                }
            }

            if (found == 0)
            {
                if (base_type->type->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class1 = (not_node_class_t *)base_type->type->value;
                    not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)field->key->value;
                    not_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", basic1->value, basic2->value);
                    goto region_error;
                }
                else if (base_type->type->kind == NODE_KIND_FUN)
                {
                    not_node_fun_t *fun1 = (not_node_fun_t *)base_type->type->value;
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)field->key->value;
                    not_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", basic1->value, basic2->value);
                    goto region_error;
                }
                else
                {
                    not_node_basic_t *basic2 = (not_node_basic_t *)field->key->value;
                    not_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", "lambda", basic2->value);
                    goto region_error;
                }
            }
        }
        else
        {
            not_node_generic_t *generic = (not_node_generic_t *)item1->value;

            not_record_t *record_field = not_execute_expression(field->key, new_strip, applicant, NULL);

            if (record_field == ERROR)
            {
                goto region_error;
            }

            if (record_field->kind != RECORD_KIND_TYPE)
            {
                not_error_type_by_node(field->key, "'%s' unsupported type: '%s'",
                                       "field", not_record_type_as_string(record_field));
                goto region_error;
            }

            if (generic->type)
            {
                not_record_t *record_gen_type = not_execute_expression(generic->type, new_strip, applicant, NULL);
                if (record_gen_type == ERROR)
                {
                    if (not_record_link_decrease(record_field) < 0)
                    {
                        return ERROR;
                    }
                    goto region_error;
                }

                int32_t r1 = not_execute_type_extends_of_type(field->key, record_field, record_gen_type, new_strip, applicant);
                if (r1 < 0)
                {
                    if (not_record_link_decrease(record_field) < 0)
                    {
                        return ERROR;
                    }
                    if (not_record_link_decrease(record_gen_type) < 0)
                    {
                        return ERROR;
                    }
                    goto region_error;
                }
                else if (r1 == 0)
                {
                    if (not_record_link_decrease(record_field) < 0)
                    {
                        return ERROR;
                    }
                    if (not_record_link_decrease(record_gen_type) < 0)
                    {
                        return ERROR;
                    }
                    not_error_type_by_node(field->key, "'%s' mismatch: '%s' and '%s'",
                                           "field", not_record_type_as_string(record_field), not_record_type_as_string(record_gen_type));
                    goto region_error;
                }

                if (not_record_link_decrease(record_gen_type) < 0)
                {
                    return ERROR;
                }
            }

            not_entry_t *entry = not_strip_variable_push(new_strip, base_type->type, item1, generic->key, record_field);
            if (entry == ERROR)
            {
                if (not_record_link_decrease(record_field) < 0)
                {
                    return ERROR;
                }
                goto region_error;
            }
            else if (entry == NULL)
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)generic->key->value;
                not_error_type_by_node(field->key, "'%s' already set", basic1->value);

                if (not_record_link_decrease(record_field) < 0)
                {
                    return ERROR;
                }
                goto region_error;
            }

            item1 = item1->next;
        }
    }

    for (; item1 != NULL; item1 = item1->next)
    {
        not_node_generic_t *generic = (not_node_generic_t *)item1->value;
        not_entry_t *entry = not_strip_variable_find(new_strip, base_type->type, generic->key);
        if (!entry)
        {
            if (generic->value)
            {
                not_record_t *record_field = not_execute_expression(generic->value, new_strip, applicant, NULL);
                if (record_field == ERROR)
                {
                    goto region_error;
                }

                if (record_field->kind != RECORD_KIND_TYPE)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)generic->key->value;
                    not_error_type_by_node(generic->value, "'%s' unsupported type: '%s'",
                                           basic1->value, not_record_type_as_string(record_field));
                    goto region_error;
                }

                not_entry_t *entry = not_strip_variable_push(new_strip, base_type->type, item1, generic->key, record_field);
                if (entry == ERROR)
                {
                    if (not_record_link_decrease(record_field) < 0)
                    {
                        return ERROR;
                    }
                    goto region_error;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)generic->key->value;
                    not_error_type_by_node(generic->key, "'%s' already set", basic1->value);

                    if (not_record_link_decrease(record_field) < 0)
                    {
                        return ERROR;
                    }
                    goto region_error;
                }
            }
            else
            {
                if (base_type->type->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class1 = (not_node_class_t *)base_type->type->value;
                    not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)generic->key->value;
                    not_error_type_by_node(node, "'%s' missing '%s' required positional field", basic1->value, basic2->value);
                    goto region_error;
                }
                else if (base_type->type->kind == NODE_KIND_FUN)
                {
                    not_node_fun_t *fun1 = (not_node_fun_t *)base_type->type->value;
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)generic->key->value;
                    not_error_type_by_node(node, "'%s' missing '%s' required positional field", basic1->value, basic2->value);
                    goto region_error;
                }
                else
                {
                    not_node_basic_t *basic2 = (not_node_basic_t *)generic->key->value;
                    not_error_type_by_node(node, "'%s' missing '%s' required positional field", "lambda", basic2->value);
                    goto region_error;
                }
                goto region_error;
            }
        }
    }

    return base;

region_error:
    if (not_record_link_decrease(base) < 0)
    {
        return ERROR;
    }
    return ERROR;

region_error_nogen:
    if (not_record_link_decrease(base) < 0)
    {
        return ERROR;
    }
    not_error_type_by_node(node, "it has no generic type");
    return ERROR;
}
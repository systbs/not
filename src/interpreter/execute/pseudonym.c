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
#include "execute.h"

sy_record_t *
sy_execute_pseudonym(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_carrier_t *carrier = (sy_node_carrier_t *)node->value;

    sy_record_t *base = sy_execute_expression(carrier->base, strip, applicant, origin);
    if (base == ERROR)
    {
        return ERROR;
    }

    if (base->kind != RECORD_KIND_TYPE)
    {
        goto region_error_nogen;
    }
    
    if (base->link == 1)
    {
        base = sy_record_copy(base);
        if (base == ERROR)
        {
            return ERROR;
        }
    }

    sy_record_type_t *base_type = (sy_record_type_t *)base->value;
    sy_strip_t *new_strip = NULL;
    sy_node_block_t *block1 = NULL;
    if (base_type->type->kind == NODE_KIND_CLASS)
    {
        sy_node_class_t *class1 = (sy_node_class_t *)base_type->type->value;
        if (!class1->generics)
        {
            goto region_error_nogen; 
        }

        new_strip = (sy_strip_t *)base_type->value;
        block1 = (sy_node_block_t *)class1->generics->value;
    }
    else
    if (base_type->type->kind == NODE_KIND_FUN)
    {
        sy_node_fun_t *fun1 = (sy_node_fun_t *)base_type->type->value;
        if (!fun1->generics)
        {
            goto region_error_nogen; 
        }

        new_strip = (sy_strip_t *)base_type->value;
        block1 = (sy_node_block_t *)fun1->generics->value;
    }
    else
    if (base_type->type->kind == NODE_KIND_LAMBDA)
    {
        sy_node_lambda_t *fun1 = (sy_node_lambda_t *)base_type->type->value;
        if (!fun1->generics || !fun1->body)
        {
            goto region_error_nogen; 
        }

        new_strip = (sy_strip_t *)base_type->value;
        block1 = (sy_node_block_t *)fun1->generics->value;
    }
    else
    {
        goto region_error_nogen;
    }
    
    sy_node_t *item1 = block1->items;

    sy_node_block_t *block2 = carrier->data->value;
    for (sy_node_t *item2 = block2->items;item2 != NULL;item2 = item2->next)
    {
        sy_node_field_t *field = (sy_node_field_t *)item2->value;

        if (!item1)
        {
            if (field->value)
            {
                if (base_type->type->kind == NODE_KIND_CLASS)
                {
                    sy_node_class_t *class1 = (sy_node_class_t *)base_type->type->value;
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)field->key->value;
                    sy_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", basic1->value, basic2->value);
                    goto region_error;
                }
                else
                if (base_type->type->kind == NODE_KIND_FUN)
                {
                    sy_node_fun_t *fun1 = (sy_node_fun_t *)base_type->type->value;
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)field->key->value;
                    sy_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", basic1->value, basic2->value);
                    goto region_error;
                }
                else
                {
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)field->key->value;
                    sy_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", "lambda", basic2->value);
                    goto region_error;
                }
            }
            else
            {
                uint64_t cnt1 = 0;
                for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                {
                    cnt1 += 1;
                }

                uint64_t cnt2 = 0;
                for (sy_node_t *item1 = block2->items;item1 != NULL;item1 = item1->next)
                {
                    cnt2 += 1;
                }

                if (base_type->type->kind == NODE_KIND_CLASS)
                {
                    sy_node_class_t *class1 = (sy_node_class_t *)base_type->type->value;
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(field->key, "'%s' takes %lld positional fields but %lld were given", basic1->value, cnt1, cnt2);
                    goto region_error;
                }
                else
                if (base_type->type->kind == NODE_KIND_FUN)
                {
                    sy_node_fun_t *fun1 = (sy_node_fun_t *)base_type->type->value;
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                    sy_error_type_by_node(field->key, "'%s' takes %lld positional fields but %lld were given", basic1->value, cnt1, cnt2);
                    goto region_error;
                }
                else
                {
                    sy_error_type_by_node(field->key, "'%s' takes %lld positional fields but %lld were given", "lambda", cnt1, cnt2);
                    goto region_error;
                }

                goto region_error;
            }
        }
        
        if (field->value)
        {
            int8_t found = 0;
            for (sy_node_t *item3 = item1;item3 != NULL;item3 = item3->next)
            {
                sy_node_generic_t *generic = (sy_node_generic_t *)item3->value;
                if (sy_execute_id_cmp(field->key, generic->key) == 1)
                {
                    found = 1;
                    sy_record_t *record_field = sy_execute_expression(field->value, new_strip, applicant, NULL);
                    if (record_field == ERROR)
                    {
                        goto region_error;
                    }

                    if (record_field->kind != RECORD_KIND_TYPE)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)field->key->value;
                        sy_error_type_by_node(field->value, "'%s' unsupported type: '%s'", 
                            basic1->value, sy_record_type_as_string(record_field));
                        goto region_error;
                    }

                    if (generic->type)
                    {
                        sy_record_t *record_gen_type = sy_execute_expression(generic->type, new_strip, applicant, NULL);
                        if (record_gen_type == ERROR)
                        {
                            if (record_field->link == 0)
                            {
                                if (sy_record_destroy(record_field) < 0)
                                {
                                    goto region_error;
                                }
                            }
                            goto region_error;
                        }

                        if (record_gen_type->kind != RECORD_KIND_TYPE)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic->key->value;
                            sy_error_type_by_node(generic->type, "'%s' unsupported type: '%s'", 
                                basic1->value, sy_record_type_as_string(record_gen_type));

                            if (record_field->link == 0)
                            {
                                if (sy_record_destroy(record_field) < 0)
                                {
                                    goto region_error;
                                }
                            }
                            
                            goto region_error;
                        }

                        int32_t r1 = sy_execute_type_check_by_type(record_field, record_gen_type, new_strip, applicant);
                        if (r1 < 0)
                        {
                            if (record_field->link == 0)
                            {
                                if (sy_record_destroy(record_field) < 0)
                                {
                                    if (record_gen_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_gen_type) < 0)
                                        {
                                            goto region_error;
                                        }
                                    }
                                    goto region_error;
                                }
                            }
                            if (record_gen_type->link == 0)
                            {
                                if (sy_record_destroy(record_gen_type) < 0)
                                {
                                    goto region_error;
                                }
                            }
                            goto region_error;
                        }
                        else
                        if (r1 == 0)
                        {
                            if (record_field->link == 0)
                            {
                                if (sy_record_destroy(record_field) < 0)
                                {
                                    if (record_gen_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_gen_type) < 0)
                                        {
                                            goto region_error;
                                        }
                                    }
                                    goto region_error;
                                }
                            }
                            if (record_gen_type->link == 0)
                            {
                                if (sy_record_destroy(record_gen_type) < 0)
                                {
                                    goto region_error;
                                }
                            }
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)field->key->value;
                            sy_error_type_by_node(field->key, "'%s' mismatch: '%s' and '%s'", 
                            basic1->value, sy_record_type_as_string(record_field), sy_record_type_as_string(record_gen_type));
                            goto region_error;
                        }

                        if (record_gen_type->link == 0)
                        {
                            if (sy_record_destroy(record_gen_type) < 0)
                            {
                                goto region_error;
                            }
                        }
                    }

                    sy_entry_t *entry = sy_strip_variable_push(new_strip, base_type->type, item3, generic->key, record_field);
                    if (entry == ERROR)
                    {
                        if (record_field->link == 0)
                        {
                            if (sy_record_destroy(record_field) < 0)
                            {
                                goto region_error;
                            }
                        }
                        goto region_error;
                    }
                    else
                    if (entry == NULL)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)generic->key->value;
                        sy_error_type_by_node(field->key, "'%s' already set", basic1->value);

                        if (record_field->link == 0)
                        {
                            if (sy_record_destroy(record_field) < 0)
                            {
                                goto region_error;
                            }
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
                    sy_node_class_t *class1 = (sy_node_class_t *)base_type->type->value;
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)field->key->value;
                    sy_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", basic1->value ,basic2->value);
                    goto region_error;
                }
                else
                if (base_type->type->kind == NODE_KIND_FUN)
                {
                    sy_node_fun_t *fun1 = (sy_node_fun_t *)base_type->type->value;
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)field->key->value;
                    sy_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", basic1->value ,basic2->value);
                    goto region_error;
                }
                else
                {
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)field->key->value;
                    sy_error_type_by_node(field->key, "'%s' got an unexpected keyword field '%s'", "lambda",basic2->value);
                    goto region_error;
                }
            }
        }
        else
        {
            sy_node_generic_t *generic = (sy_node_generic_t *)item1->value;

            sy_record_t *record_field = sy_execute_expression(field->key, new_strip, applicant, NULL);

            if (record_field == ERROR)
            {
                goto region_error;
            }

            if (record_field->kind != RECORD_KIND_TYPE)
            {
                sy_error_type_by_node(field->key, "'%s' unsupported type: '%s'", 
                    "field", sy_record_type_as_string(record_field));
                goto region_error;
            }

            if (generic->type)
            {
                sy_record_t *record_gen_type = sy_execute_expression(generic->type, new_strip, applicant, NULL);
                if (record_gen_type == ERROR)
                {
                    if (record_field->link == 0)
                    {
                        if (sy_record_destroy(record_field) < 0)
                        {
                            goto region_error;
                        }
                    }
                    goto region_error;
                }

                int32_t r1 = sy_execute_type_check_by_type(record_field, record_gen_type, new_strip, applicant);
                if (r1 < 0)
                {
                    if (record_field->link == 0)
                    {
                        if (sy_record_destroy(record_field) < 0)
                        {
                            if (record_gen_type->link == 0)
                            {
                                if (sy_record_destroy(record_gen_type) < 0)
                                {
                                    goto region_error;
                                }
                            }
                            goto region_error;
                        }
                    }
                    if (record_gen_type->link == 0)
                    {
                        if (sy_record_destroy(record_gen_type) < 0)
                        {
                            goto region_error;
                        }
                    }
                    goto region_error;
                }
                else
                if (r1 == 0)
                {
                    if (record_field->link == 0)
                    {
                        if (sy_record_destroy(record_field) < 0)
                        {
                            if (record_gen_type->link == 0)
                            {
                                if (sy_record_destroy(record_gen_type) < 0)
                                {
                                    goto region_error;
                                }
                            }
                            goto region_error;
                        }
                    }
                    if (record_gen_type->link == 0)
                    {
                        if (sy_record_destroy(record_gen_type) < 0)
                        {
                            goto region_error;
                        }
                    }
                    sy_error_type_by_node(field->key, "'%s' mismatch: '%s' and '%s'", 
                    "field", sy_record_type_as_string(record_field), sy_record_type_as_string(record_gen_type));
                    goto region_error;
                }

                if (record_gen_type->link == 0)
                {
                    if (sy_record_destroy(record_gen_type) < 0)
                    {
                        goto region_error;
                    }
                }
            }

            sy_entry_t *entry = sy_strip_variable_push(new_strip, base_type->type, item1, generic->key, record_field);
            if (entry == ERROR)
            {
                if (record_field->link == 0)
                {
                    if (sy_record_destroy(record_field) < 0)
                    {
                        goto region_error;
                    }
                }
                goto region_error;
            }
            else
            if (entry == NULL)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)generic->key->value;
                sy_error_type_by_node(field->key, "'%s' already set", basic1->value);

                if (record_field->link == 0)
                {
                    if (sy_record_destroy(record_field) < 0)
                    {
                        goto region_error;
                    }
                }
                goto region_error;
            }

            item1 = item1->next;
        }
    }

    for (;item1 != NULL;item1 = item1->next)
    {
        sy_node_generic_t *generic = (sy_node_generic_t *)item1->value;
        sy_entry_t *entry = sy_strip_variable_find(new_strip, base_type->type, generic->key);
        if (!entry)
        {
            if (generic->value)
            {
                sy_record_t *record_field = sy_execute_expression(generic->value, new_strip, applicant, NULL);
                if (record_field == ERROR)
                {
                    goto region_error;
                }

                if (record_field->kind != RECORD_KIND_TYPE)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)generic->key->value;
                    sy_error_type_by_node(generic->value, "'%s' unsupported type: '%s'", 
                        basic1->value, sy_record_type_as_string(record_field));
                    goto region_error;
                }

                sy_entry_t *entry = sy_strip_variable_push(new_strip, base_type->type, item1, generic->key, record_field);
                if (entry == ERROR)
                {
                    if (record_field->link == 0)
                    {
                        if (sy_record_destroy(record_field) < 0)
                        {
                            goto region_error;
                        }
                    }
                    goto region_error;
                }
                else
                if (entry == NULL)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)generic->key->value;
                    sy_error_type_by_node(generic->key, "'%s' already set", basic1->value);

                    if (record_field->link == 0)
                    {
                        if (sy_record_destroy(record_field) < 0)
                        {
                            goto region_error;
                        }
                    }
                    goto region_error;
                }
            }
            else
            {
                if (base_type->type->kind == NODE_KIND_CLASS)
                {
                    sy_node_class_t *class1 = (sy_node_class_t *)base_type->type->value;
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)generic->key->value;
                    sy_error_type_by_node(node, "'%s' missing '%s' required positional field", basic1->value, basic2->value);
                    goto region_error;
                }
                else
                if (base_type->type->kind == NODE_KIND_FUN)
                {
                    sy_node_fun_t *fun1 = (sy_node_fun_t *)base_type->type->value;
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)generic->key->value;
                    sy_error_type_by_node(node, "'%s' missing '%s' required positional field", basic1->value, basic2->value);
                    goto region_error;
                }
                else
                {
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)generic->key->value;
                    sy_error_type_by_node(node, "'%s' missing '%s' required positional field", "lambda", basic2->value);
                    goto region_error;
                }
                goto region_error;
            }
        }
    }

    return base;

    region_error:
    if (base->link == 0)
    {
        if (sy_record_destroy(base) < 0)
        {
            return ERROR;
        }
    }
    return ERROR;

    region_error_nogen:
    if (base->link == 0)
    {
        if (sy_record_destroy(base) < 0)
        {
            return ERROR;
        }
    }
    sy_error_type_by_node(node, "it has no generic type");
    return ERROR;
}
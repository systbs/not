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
#include "../../config.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

static int32_t
parameters_substitute(sy_node_t *base, sy_node_t *node, sy_strip_t *strip, sy_node_t *parameters, sy_node_t *arguments, sy_node_t *applicant)
{
    if (arguments)
    {
        sy_node_block_t *block1 = (sy_node_block_t *)arguments->value;

        if (!parameters)
        {
            uint64_t cnt1 = 0;
            for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
            {
                cnt1 += 1;
            }

            if (node->kind == NODE_KIND_FUN)
            {
                sy_node_fun_t *fun1 = (sy_node_fun_t *)node->value;
                sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                sy_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", basic1->value, 0, cnt1);
                return -1;
            }
            else
            {
                sy_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "lambda", 0, cnt1);
                return -1;
            }
        }
    }
    
    if (parameters)
    {
        sy_node_block_t *block1 = (sy_node_block_t *)parameters->value;
        sy_node_t *item1 = block1->items;
        if (arguments)
        {
            sy_node_block_t *block2 = (sy_node_block_t *)arguments->value;
            for (sy_node_t *item2 = block2->items;item2 != NULL;)
            {
                sy_node_argument_t *argument = (sy_node_argument_t *)item2->value;

                if (!item1)
                {
                    if (argument->value)
                    {
                        if (node->kind == NODE_KIND_FUN)
                        {
                            sy_node_fun_t *fun1 = (sy_node_fun_t *)node->value;
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                            sy_node_basic_t *basic2 = (sy_node_basic_t *)argument->key->value;
                            sy_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", basic1->value, basic2->value);
                            return -1;
                        }
                        else
                        {
                            sy_node_basic_t *basic2 = (sy_node_basic_t *)argument->key->value;
                            sy_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", "lambda", basic2->value);
                            return -1;
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

                        if (node->kind == NODE_KIND_FUN)
                        {
                            sy_node_fun_t *fun1 = (sy_node_fun_t *)node->value;
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                            sy_error_type_by_node(argument->key, "'%s' takes %lld positional arguments but %lld were given", basic1->value, cnt1, cnt2);
                            return -1;
                        }
                        else
                        {
                            sy_error_type_by_node(argument->key, "'%s' takes %lld positional arguments but %lld were given", "lambda", cnt1, cnt2);
                            return -1;
                        }

                        return -1;
                    }
                }

                if (argument->value)
                {
                    sy_node_parameter_t *parameter = (sy_node_parameter_t *)item1->value;
                    if ((parameter->flag & SYNTAX_MODIFIER_KWARG) == SYNTAX_MODIFIER_KWARG)
                    {
                        sy_record_object_t *object = NULL;

                        for (;item2 != NULL;item2 = item2->next)
                        {
                            argument = (sy_node_argument_t *)item2->value;
                            if (!argument->value)
                            {
                                break;
                            }

                            sy_record_t *record_arg = sy_execute_expression(argument->value, strip, applicant, NULL);
                            if (record_arg == ERROR)
                            {
                                return -1;
                            }

                            if (parameter->type)
                            {
                                sy_record_t *record_param_type = sy_execute_expression(parameter->type, strip, applicant, NULL);
                                if (record_param_type == ERROR)
                                {
                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    return -1;
                                }

                                if (record_param_type->kind != RECORD_KIND_TYPE)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                    sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                        basic1->value, sy_record_type_as_string(record_param_type));
                                    
                                    if (record_param_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_param_type) < 0)
                                        {
                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            return -1;
                                        }
                                    }

                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }

                                    return -1;
                                }

                                int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                                if (r1 < 0)
                                {
                                    if (record_param_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_param_type) < 0)
                                        {
                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            return -1;
                                        }
                                    }

                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                    {
                                        if (object == NULL)
                                        {
                                            sy_node_basic_t *basic1 = (sy_node_basic_t *)argument->key->value;
                                            sy_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'", 
                                                basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));
                                            
                                            if (record_param_type->link == 0)
                                            {
                                                if (sy_record_destroy(record_param_type) < 0)
                                                {
                                                    if (record_arg->link == 0)
                                                    {
                                                        if (sy_record_destroy(record_arg) < 0)
                                                        {
                                                            return -1;
                                                        }
                                                    }
                                                    return -1;
                                                }
                                            }

                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            
                                            return -1;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (record_arg->link == 1)
                                        {
                                            record_arg = sy_record_copy(record_arg);
                                        }

                                        sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                        if (record_arg2 == ERROR)
                                        {
                                            if (record_param_type->link == 0)
                                            {
                                                if (sy_record_destroy(record_param_type) < 0)
                                                {
                                                    if (record_arg->link == 0)
                                                    {
                                                        if (sy_record_destroy(record_arg) < 0)
                                                        {
                                                            return -1;
                                                        }
                                                    }
                                                    return -1;
                                                }
                                            }

                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            
                                            return -1;
                                        }
                                        else
                                        if (record_arg2 == NULL)
                                        {
                                            if (object == NULL)
                                            {
                                                sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                                sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                                    basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                                                if (record_param_type->link == 0)
                                                {
                                                    if (sy_record_destroy(record_param_type) < 0)
                                                    {
                                                        if (record_arg->link == 0)
                                                        {
                                                            if (sy_record_destroy(record_arg) < 0)
                                                            {
                                                                return -1;
                                                            }
                                                        }
                                                        return -1;
                                                    }
                                                }

                                                if (record_arg->link == 0)
                                                {
                                                    if (sy_record_destroy(record_arg) < 0)
                                                    {
                                                        return -1;
                                                    }
                                                }
                                                
                                                return -1;
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }

                                        record_arg = record_arg2;
                                    }
                                }

                                if (record_param_type->link == 0)
                                {
                                    if (sy_record_destroy(record_param_type) < 0)
                                    {
                                        return -1;
                                    }
                                }
                            }

                            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                            {
                                if (record_arg->link == 1)
                                {
                                    record_arg = sy_record_copy(record_arg);
                                }

                                record_arg->link = 1;
                            }

                            sy_record_object_t *object2 = sy_record_make_object(argument->key, record_arg, object);
                            if (object2 == ERROR)
                            {
                                if (record_arg->link == 0)
                                {
                                    if (sy_record_destroy(record_arg) < 0)
                                    {
                                        if (object)
                                        {
                                            if (sy_record_object_destroy(object) < 0)
                                            {
                                                return -1;
                                            }
                                        }
                                        return -1;
                                    }
                                }

                                if (object)
                                {
                                    if (sy_record_object_destroy(object) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                return -1;
                            }

                            object = object2;
                        }

                        sy_record_t *record_arg = sy_record_create(RECORD_KIND_OBJECT, object);
                        if (record_arg == ERROR)
                        {
                            if (object)
                            {
                                if (sy_record_object_destroy(object) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }

                        sy_entry_t *entry = sy_strip_input_push(strip, node, item1, parameter->key, record_arg);
                        if (entry == ERROR)
                        {
                            if (record_arg->link == 0)
                            {
                                if (sy_record_destroy(record_arg) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }

                        item1 = item1->next;
                        continue;
                    }
                    else
                    {
                        int8_t found = 0;
                        for (sy_node_t *item3 = item1;item3 != NULL;item3 = item3->next)
                        {
                            parameter = (sy_node_parameter_t *)item3->value;
                            if (sy_execute_id_cmp(argument->key, parameter->key) == 1)
                            {
                                found = 1;

                                sy_record_t *record_arg = sy_execute_expression(argument->value, strip, applicant, NULL);
                                if (record_arg == ERROR)
                                {
                                    return -1;
                                }

                                if (parameter->type)
                                {
                                    sy_record_t *record_param_type = sy_execute_expression(parameter->type, strip, applicant, NULL);
                                    if (record_param_type == ERROR)
                                    {
                                        if (record_arg->link == 0)
                                        {
                                            if (sy_record_destroy(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                        }
                                        return -1;
                                    }

                                    if (record_param_type->kind != RECORD_KIND_TYPE)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                        sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                            basic1->value, sy_record_type_as_string(record_param_type));
                                        
                                        if (record_param_type->link == 0)
                                        {
                                            if (sy_record_destroy(record_param_type) < 0)
                                            {
                                                if (record_arg->link == 0)
                                                {
                                                    if (sy_record_destroy(record_arg) < 0)
                                                    {
                                                        return -1;
                                                    }
                                                }
                                                return -1;
                                            }
                                        }

                                        if (record_arg->link == 0)
                                        {
                                            if (sy_record_destroy(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                        }

                                        return -1;
                                    }

                                    int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                                    if (r1 < 0)
                                    {
                                        if (record_param_type->link == 0)
                                        {
                                            if (sy_record_destroy(record_param_type) < 0)
                                            {
                                                if (record_arg->link == 0)
                                                {
                                                    if (sy_record_destroy(record_arg) < 0)
                                                    {
                                                        return -1;
                                                    }
                                                }
                                                return -1;
                                            }
                                        }

                                        if (record_arg->link == 0)
                                        {
                                            if (sy_record_destroy(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                        }
                                        
                                        return -1;
                                    }
                                    else
                                    if (r1 == 0)
                                    {
                                        if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                        {
                                            sy_node_basic_t *basic1 = (sy_node_basic_t *)argument->key->value;
                                            sy_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'", 
                                                basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));
                                            
                                            if (record_param_type->link == 0)
                                            {
                                                if (sy_record_destroy(record_param_type) < 0)
                                                {
                                                    if (record_arg->link == 0)
                                                    {
                                                        if (sy_record_destroy(record_arg) < 0)
                                                        {
                                                            return -1;
                                                        }
                                                    }
                                                    return -1;
                                                }
                                            }

                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            
                                            return -1;
                                        }
                                        else
                                        {
                                            if (record_arg->link == 1)
                                            {
                                                record_arg = sy_record_copy(record_arg);
                                            }

                                            sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                            if (record_arg2 == ERROR)
                                            {
                                                if (record_param_type->link == 0)
                                                {
                                                    if (sy_record_destroy(record_param_type) < 0)
                                                    {
                                                        if (record_arg->link == 0)
                                                        {
                                                            if (sy_record_destroy(record_arg) < 0)
                                                            {
                                                                return -1;
                                                            }
                                                        }
                                                        return -1;
                                                    }
                                                }

                                                if (record_arg->link == 0)
                                                {
                                                    if (sy_record_destroy(record_arg) < 0)
                                                    {
                                                        return -1;
                                                    }
                                                }
                                                
                                                return -1;
                                            }
                                            else
                                            if (record_arg2 == NULL)
                                            {
                                                sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                                sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                                    basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                                                if (record_param_type->link == 0)
                                                {
                                                    if (sy_record_destroy(record_param_type) < 0)
                                                    {
                                                        if (record_arg->link == 0)
                                                        {
                                                            if (sy_record_destroy(record_arg) < 0)
                                                            {
                                                                return -1;
                                                            }
                                                        }
                                                        return -1;
                                                    }
                                                }

                                                if (record_arg->link == 0)
                                                {
                                                    if (sy_record_destroy(record_arg) < 0)
                                                    {
                                                        return -1;
                                                    }
                                                }
                                                
                                                return -1;
                                            }

                                            record_arg = record_arg2;
                                        }
                                    }

                                    if (record_param_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_param_type) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                }

                                if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                                {
                                    if (record_arg->link == 1)
                                    {
                                        record_arg = sy_record_copy(record_arg);
                                    }

                                    record_arg->link = 1;
                                }

                                sy_entry_t *entry = sy_strip_input_push(strip, node, item3, parameter->key, record_arg);
                                if (entry == ERROR)
                                {
                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    return -1;
                                }

                                break;
                            }
                        }

                        if (found == 0)
                        {
                            if (node->kind == NODE_KIND_FUN)
                            {
                                sy_node_fun_t *fun1 = (sy_node_fun_t *)node->value;
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                                sy_node_basic_t *basic2 = (sy_node_basic_t *)argument->key->value;
                                sy_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", basic1->value ,basic2->value);
                                return -1;
                            }
                            else
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)argument->key->value;
                                sy_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", "lambda" ,basic1->value);
                                return -1;
                            }
                        }
                    }
                }
                else
                {
                    sy_node_parameter_t *parameter = (sy_node_parameter_t *)item1->value;
                    if ((parameter->flag & SYNTAX_MODIFIER_KARG) == SYNTAX_MODIFIER_KARG)
                    {
                        sy_record_tuple_t *tuple = NULL;

                        for (;item2 != NULL;item2 = item2->next)
                        {
                            argument = (sy_node_argument_t *)item2->value;

                            sy_record_t *record_arg = sy_execute_expression(argument->key, strip, applicant, NULL);
                            if (record_arg == ERROR)
                            {
                                return -1;
                            }

                            if (parameter->type)
                            {
                                sy_record_t *record_param_type = sy_execute_expression(parameter->type, strip, applicant, NULL);
                                if (record_param_type == ERROR)
                                {
                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    return -1;
                                }

                                if (record_param_type->kind != RECORD_KIND_TYPE)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                    sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                        basic1->value, sy_record_type_as_string(record_param_type));
                                    
                                    if (record_param_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_param_type) < 0)
                                        {
                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            return -1;
                                        }
                                    }

                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }

                                    return -1;
                                }

                                int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                                if (r1 < 0)
                                {
                                    if (record_param_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_param_type) < 0)
                                        {
                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            return -1;
                                        }
                                    }

                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                    {
                                        if (tuple == NULL)
                                        {
                                            sy_node_basic_t *basic1 = (sy_node_basic_t *)argument->key->value;
                                            sy_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'", 
                                                basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));
                                            
                                            if (record_param_type->link == 0)
                                            {
                                                if (sy_record_destroy(record_param_type) < 0)
                                                {
                                                    if (record_arg->link == 0)
                                                    {
                                                        if (sy_record_destroy(record_arg) < 0)
                                                        {
                                                            return -1;
                                                        }
                                                    }
                                                    return -1;
                                                }
                                            }

                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            
                                            return -1;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (record_arg->link == 1)
                                        {
                                            record_arg = sy_record_copy(record_arg);
                                        }

                                        sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                        if (record_arg2 == ERROR)
                                        {
                                            if (record_param_type->link == 0)
                                            {
                                                if (sy_record_destroy(record_param_type) < 0)
                                                {
                                                    if (record_arg->link == 0)
                                                    {
                                                        if (sy_record_destroy(record_arg) < 0)
                                                        {
                                                            return -1;
                                                        }
                                                    }
                                                    return -1;
                                                }
                                            }

                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            
                                            return -1;
                                        }
                                        else
                                        if (record_arg2 == NULL)
                                        {
                                            if (tuple == NULL)
                                            {
                                                sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                                sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                                    basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                                                if (record_param_type->link == 0)
                                                {
                                                    if (sy_record_destroy(record_param_type) < 0)
                                                    {
                                                        if (record_arg->link == 0)
                                                        {
                                                            if (sy_record_destroy(record_arg) < 0)
                                                            {
                                                                return -1;
                                                            }
                                                        }
                                                        return -1;
                                                    }
                                                }

                                                if (record_arg->link == 0)
                                                {
                                                    if (sy_record_destroy(record_arg) < 0)
                                                    {
                                                        return -1;
                                                    }
                                                }
                                                
                                                return -1;
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }

                                        record_arg = record_arg2;
                                    }
                                }

                                if (record_param_type->link == 0)
                                {
                                    if (sy_record_destroy(record_param_type) < 0)
                                    {
                                        return -1;
                                    }
                                }
                            }

                            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                            {
                                if (record_arg->link == 1)
                                {
                                    record_arg = sy_record_copy(record_arg);
                                }

                                record_arg->link = 1;
                            }

                            sy_record_tuple_t *tuple2 = sy_record_make_tuple(record_arg, tuple);
                            if (tuple2 == ERROR)
                            {
                                if (record_arg->link == 0)
                                {
                                    if (sy_record_destroy(record_arg) < 0)
                                    {
                                        if (tuple)
                                        {
                                            if (sy_record_tuple_destroy(tuple) < 0)
                                            {
                                                return -1;
                                            }
                                        }
                                        return -1;
                                    }
                                }

                                if (tuple)
                                {
                                    if (sy_record_tuple_destroy(tuple) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                return -1;
                            }

                            tuple = tuple2;
                        }

                        sy_record_t *record_arg = sy_record_create(RECORD_KIND_TUPLE, tuple);
                        if (record_arg == ERROR)
                        {
                            if (tuple)
                            {
                                if (sy_record_tuple_destroy(tuple) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }

                        sy_entry_t *entry = sy_strip_input_push(strip, node, item1, parameter->key, record_arg);
                        if (entry == ERROR)
                        {
                            if (record_arg->link == 0)
                            {
                                if (sy_record_destroy(record_arg) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }

                        item1 = item1->next;
                        continue;
                    }
                    else
                    if ((parameter->flag & SYNTAX_MODIFIER_KWARG) == SYNTAX_MODIFIER_KWARG)
                    {
                        sy_record_t *record_arg = sy_execute_expression(argument->key, strip, applicant, NULL);
                        if (record_arg == ERROR)
                        {
                            return -1;
                        }

                        sy_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'", 
                            "argument", sy_record_type_as_string(record_arg), "kwarg");

                        if (record_arg->link == 0)
                        {
                            if (sy_record_destroy(record_arg) < 0)
                            {
                                return -1;
                            }
                        }
                        return -1;
                    }
                    else
                    {
                        sy_record_t *record_arg = sy_execute_expression(argument->key, strip, applicant, NULL);
                        if (record_arg == ERROR)
                        {
                            return -1;
                        }
                        
                        if (parameter->type)
                        {
                            sy_record_t *record_param_type = sy_execute_expression(parameter->type, strip, applicant, NULL);
                            if (record_param_type == ERROR)
                            {
                                if (record_arg->link == 0)
                                {
                                    if (sy_record_destroy(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                return -1;
                            }

                            if (record_param_type->kind != RECORD_KIND_TYPE)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                    basic1->value, sy_record_type_as_string(record_param_type));
                                
                                if (record_param_type->link == 0)
                                {
                                    if (sy_record_destroy(record_param_type) < 0)
                                    {
                                        if (record_arg->link == 0)
                                        {
                                            if (sy_record_destroy(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                        }
                                        return -1;
                                    }
                                }

                                if (record_arg->link == 0)
                                {
                                    if (sy_record_destroy(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                }

                                return -1;
                            }

                            int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                            if (r1 < 0)
                            {
                                if (record_param_type->link == 0)
                                {
                                    if (sy_record_destroy(record_param_type) < 0)
                                    {
                                        if (record_arg->link == 0)
                                        {
                                            if (sy_record_destroy(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                        }
                                        return -1;
                                    }
                                }

                                if (record_arg->link == 0)
                                {
                                    if (sy_record_destroy(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                
                                return -1;
                            }
                            else
                            if (r1 == 0)
                            {
                                if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                {
                                    sy_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'", 
                                        "argument", sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));
                                    
                                    if (record_param_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_param_type) < 0)
                                        {
                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            return -1;
                                        }
                                    }

                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    
                                    return -1;
                                }
                                else
                                {
                                    if (record_arg->link == 1)
                                    {
                                        record_arg = sy_record_copy(record_arg);
                                    }

                                    sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                    if (record_arg2 == ERROR)
                                    {
                                        if (record_param_type->link == 0)
                                        {
                                            if (sy_record_destroy(record_param_type) < 0)
                                            {
                                                if (record_arg->link == 0)
                                                {
                                                    if (sy_record_destroy(record_arg) < 0)
                                                    {
                                                        return -1;
                                                    }
                                                }
                                                return -1;
                                            }
                                        }

                                        if (record_arg->link == 0)
                                        {
                                            if (sy_record_destroy(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                        }
                                        
                                        return -1;
                                    }
                                    else
                                    if (record_arg2 == NULL)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                        sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                            basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                                        if (record_param_type->link == 0)
                                        {
                                            if (sy_record_destroy(record_param_type) < 0)
                                            {
                                                if (record_arg->link == 0)
                                                {
                                                    if (sy_record_destroy(record_arg) < 0)
                                                    {
                                                        return -1;
                                                    }
                                                }
                                                return -1;
                                            }
                                        }

                                        if (record_arg->link == 0)
                                        {
                                            if (sy_record_destroy(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                        }
                                        
                                        return -1;
                                    }

                                    record_arg = record_arg2;
                                }
                            }

                            if (record_param_type->link == 0)
                            {
                                if (sy_record_destroy(record_param_type) < 0)
                                {
                                    return -1;
                                }
                            }
                        }
                        
                        if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                        {
                            if (record_arg->link == 1)
                            {
                                record_arg = sy_record_copy(record_arg);
                            }

                            record_arg->link = 1;
                        }

                        sy_entry_t *entry = sy_strip_input_push(strip, node, item1, parameter->key, record_arg);
                        if (entry == ERROR)
                        {
                            if (record_arg->link == 0)
                            {
                                if (sy_record_destroy(record_arg) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }

                        item1 = item1->next;
                    }
                }

                item2 = item2->next;
            }
        }
        
        for (;item1 != NULL;item1 = item1->next)
        {
            sy_node_parameter_t *parameter = (sy_node_parameter_t *)item1->value;
            sy_entry_t *entry = sy_strip_input_find(strip, node, parameter->key);
            if (!entry)
            {
                if (parameter->value)
                {
                    sy_record_t *record_arg = sy_execute_expression(parameter->value, strip, applicant, NULL);
                    if (record_arg == ERROR)
                    {
                        return -1;
                    }

                    if (parameter->type)
                    {
                        sy_record_t *record_param_type = sy_execute_expression(parameter->type, strip, applicant, NULL);
                        if (record_param_type == ERROR)
                        {
                            if (record_arg->link == 0)
                            {
                                if (sy_record_destroy(record_arg) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }

                        if (record_param_type->kind != RECORD_KIND_TYPE)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                            sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                basic1->value, sy_record_type_as_string(record_param_type));
                            
                            if (record_param_type->link == 0)
                            {
                                if (sy_record_destroy(record_param_type) < 0)
                                {
                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    return -1;
                                }
                            }

                            if (record_arg->link == 0)
                            {
                                if (sy_record_destroy(record_arg) < 0)
                                {
                                    return -1;
                                }
                            }

                            return -1;
                        }

                        int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                        if (r1 < 0)
                        {
                            if (record_param_type->link == 0)
                            {
                                if (sy_record_destroy(record_param_type) < 0)
                                {
                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    return -1;
                                }
                            }

                            if (record_arg->link == 0)
                            {
                                if (sy_record_destroy(record_arg) < 0)
                                {
                                    return -1;
                                }
                            }
                            
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                    basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));
                                
                                if (record_param_type->link == 0)
                                {
                                    if (sy_record_destroy(record_param_type) < 0)
                                    {
                                        if (record_arg->link == 0)
                                        {
                                            if (sy_record_destroy(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                        }
                                        return -1;
                                    }
                                }

                                if (record_arg->link == 0)
                                {
                                    if (sy_record_destroy(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                
                                return -1;
                            }
                            else
                            {
                                if (record_arg->link == 1)
                                {
                                    record_arg = sy_record_copy(record_arg);
                                }

                                sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                if (record_arg2 == ERROR)
                                {
                                    if (record_param_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_param_type) < 0)
                                        {
                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            return -1;
                                        }
                                    }

                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    
                                    return -1;
                                }
                                else
                                if (record_arg2 == NULL)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                    sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                        basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                                    if (record_param_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_param_type) < 0)
                                        {
                                            if (record_arg->link == 0)
                                            {
                                                if (sy_record_destroy(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                            }
                                            return -1;
                                        }
                                    }

                                    if (record_arg->link == 0)
                                    {
                                        if (sy_record_destroy(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                    }
                                    
                                    return -1;
                                }

                                record_arg = record_arg2;
                            }
                        }

                        if (record_param_type->link == 0)
                        {
                            if (sy_record_destroy(record_param_type) < 0)
                            {
                                return -1;
                            }
                        }
                    }

                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (record_arg->link == 1)
                        {
                            record_arg = sy_record_copy(record_arg);
                        }

                        record_arg->link = 1;
                    }

                    sy_entry_t *entry2 = sy_strip_input_push(strip, node, item1, parameter->key, record_arg);
                    if (entry2 == ERROR)
                    {
                        if (record_arg->link == 0)
                        {
                            if (sy_record_destroy(record_arg) < 0)
                            {
                                return -1;
                            }
                        }
                        return -1;
                    }
                }
                else
                {
                    if (node->kind == NODE_KIND_FUN)
                    {
                        sy_node_fun_t *fun1 = (sy_node_fun_t *)node->value;
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                        sy_node_basic_t *basic2 = (sy_node_basic_t *)parameter->key->value;
                        sy_error_type_by_node(base, "'%s' missing '%s' required positional argument", basic1->value, basic2->value);
                        return -1;
                    }
                    else
                    {
                        sy_node_basic_t *basic2 = (sy_node_basic_t *)parameter->key->value;
                        sy_error_type_by_node(base, "'%s' missing '%s' required positional argument", "lambda", basic2->value);
                        return -1;
                    }
                }
            }
        }
    }

    return 0;
}

static int32_t
heritages_substitute(sy_node_t *node, sy_strip_t *strip, sy_node_t *heritages, sy_node_t *applicant)
{
    sy_node_class_t *class1 = (sy_node_class_t *)node->value;

    sy_node_block_t *block = (sy_node_block_t *)class1->heritages->value;
    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        sy_node_heritage_t *heritage = (sy_node_heritage_t *)item->value;
        
        sy_record_t *record_heritage = sy_execute_expression(heritage->type, strip, applicant, NULL);
        if (record_heritage == ERROR)
        {
            return -1;
        }

        if (record_heritage->kind != RECORD_KIND_TYPE)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage->key->value;
            sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
            sy_error_type_by_node(heritage->key, "'%s' unexpected type as heritage '%s'", 
                basic2->value, basic1->value);

            if (record_heritage->link == 0)
            {
                if (sy_record_destroy(record_heritage) < 0)
                {
                    return -1;
                }
            }
            return -1;
        }

        sy_record_type_t *record_type = (sy_record_type_t *)record_heritage->value;
        sy_node_t *type = record_type->type;

        if (type->kind == NODE_KIND_CLASS)
        {
            sy_strip_t *new_strip = sy_strip_copy((sy_strip_t *)record_type->value);
            if (new_strip == ERROR)
            {
                if (record_heritage->link == 0)
                {
                    if (sy_record_destroy(record_heritage) < 0)
                    {
                        return -1;
                    }
                }
                return -1;
            }

            sy_record_t *record = sy_record_make_struct(type, new_strip);
            if (record == ERROR)
            {
                if (sy_strip_destroy(new_strip) < 0)
                {
                    if (record_heritage->link == 0)
                    {
                        if (sy_record_destroy(record_heritage) < 0)
                        {
                            return -1;
                        }
                    }
                    return -1;
                }
                if (record_heritage->link == 0)
                {
                    if (sy_record_destroy(record_heritage) < 0)
                    {
                        return -1;
                    }
                }
                return -1;
            }

            if (record_heritage->link == 0)
            {
                if (sy_record_destroy(record_heritage) < 0)
                {
                    return -1;
                }
            }

            sy_entry_t *entry = sy_strip_variable_push(strip, node, item, heritage->key, record);
            if (entry == ERROR)
            {
                return -1;
            }
            else
            if (entry == NULL)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage->key->value;
                sy_error_type_by_node(heritage->key, "'%s' already set", basic1->value);
                return -1;
            }
        }
        else
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage->key->value;
            sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
            sy_error_type_by_node(heritage->key, "'%s' unexpected type as heritage '%s'", 
                basic2->value, basic1->value);

            if (record_heritage->link == 0)
            {
                if (sy_record_destroy(record_heritage) < 0)
                {
                    return -1;
                }
            }
            return -1;
        }
    }

    return 0;
}


sy_record_t *
sy_execute_call(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_carrier_t *carrier = (sy_node_carrier_t *)node->value;

    sy_record_t *base = sy_execute_expression(carrier->base, strip, applicant, origin);
    if (base == ERROR)
    {
        return ERROR;
    }
    
    if (base->kind == RECORD_KIND_TYPE)
    {
        sy_record_type_t *record_type = (sy_record_type_t *)base->value;
        sy_node_t *type = record_type->type;
        if (type->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)type->value;
            
            sy_strip_t *new_strip_class = sy_strip_copy((sy_strip_t *)record_type->value);
            if (new_strip_class == ERROR)
            {
                return ERROR;
            }

            if (class1->heritages)
            {
                if (heritages_substitute(type, new_strip_class, class1->heritages, applicant) < 0)
                {
                    if (sy_strip_destroy(new_strip_class) < 0)
                    {
                        if (base->link == 0)
                        {
                            if (sy_record_destroy(base) < 0)
                            {
                                return ERROR;
                            }
                        }
                        return ERROR;
                    }
                    if (base->link == 0)
                    {
                        if (sy_record_destroy(base) < 0)
                        {
                            return ERROR;
                        }
                    }
                    return ERROR;
                }
            }

            for (sy_node_t *item = class1->block; item != NULL; item = item->next)
            {
                if (item->kind == NODE_KIND_FUN)
                {
                    sy_node_fun_t *fun1 = (sy_node_fun_t *)item->value;
                    if (sy_execute_id_strcmp(fun1->key, CONSTRUCTOR_STR) == 1)
                    {
                        sy_strip_t *new_strip_fun = sy_strip_create(new_strip_class);
                        if (new_strip_fun == ERROR)
                        {
                            if (sy_strip_destroy(new_strip_class) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }

                        if (parameters_substitute(carrier->base, item, new_strip_fun, fun1->parameters, carrier->data, applicant) < 0)
                        {
                            if (sy_strip_destroy(new_strip_fun) < 0)
                            {
                                if (sy_strip_destroy(new_strip_class) < 0)
                                {
                                    return ERROR;
                                }
                                return ERROR;
                            }
                            if (sy_strip_destroy(new_strip_class) < 0)
                            {
                                return ERROR;
                            }
                        }
                    }
                }
            }

            return base;
        }
        else
        {
            sy_error_type_by_node(carrier->base, "'%s' object is not callable", 
                sy_node_kind_as_string(type));

            if (base->link == 0)
            {
                if (sy_record_destroy(base) < 0)
                {
                    return ERROR;
                }
            }

            return ERROR;
        }
    }

    sy_error_type_by_node(node, "call implement not support");
    return ERROR;
}
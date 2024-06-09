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
#include "../../interpreter.h"
#include "../../thread.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

static sy_record_t *
sy_execute_provide_class(sy_strip_t *strip, sy_node_t *node, sy_node_t *applicant);

int32_t 
sy_execute_parameters_substitute_by_one_argument(sy_node_t *base, sy_node_t *scope, sy_strip_t *strip, sy_node_t *parameters, sy_record_t *arg, sy_node_t *applicant)
{
    if (parameters)
    {
        sy_node_block_t *block1 = (sy_node_block_t *)parameters->value;
        sy_node_t *item1 = block1->items;
        
        sy_node_parameter_t *parameter = (sy_node_parameter_t *)item1->value;
        if ((parameter->flag & SYNTAX_MODIFIER_KARG) == SYNTAX_MODIFIER_KARG)
        {
            sy_record_tuple_t *tuple = NULL;

            if (arg)
            {
                sy_record_t *record_arg = sy_record_copy(arg);
                if (record_arg == ERROR)
                {
                    return -1;
                }

                if (parameter->type)
                {
                    sy_record_t *record_param_type = sy_execute_expression(parameter->type, strip, applicant, NULL);
                    if (record_param_type == ERROR)
                    {
                        record_arg->link -= 1;
                        return -1;
                    }

                    if (record_param_type->kind != RECORD_KIND_TYPE)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                        sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                            basic1->value, sy_record_type_as_string(record_param_type));
                        
                        record_param_type->link -= 1;

                        record_arg->link -= 1;

                        return -1;
                    }

                    int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                    if (r1 < 0)
                    {
                        record_param_type->link -= 1;

                        record_arg->link -= 1;
                        
                        return -1;
                    }
                    else
                    if (r1 == 0)
                    {
                        if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                        {
                            if (tuple == NULL)
                            {
                                sy_error_type_by_node(base, "'%s' mismatch: '%s' and '%s'", 
                                    "argument", sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));
                                
                                record_param_type->link -= 1;

                                record_arg->link -= 1;

                                return -1;
                            }
                        }
                        else
                        {
                            if (record_arg->link > 1)
                            {
                                record_arg = sy_record_copy(record_arg);
                            }

                            sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                            if (record_arg2 == ERROR)
                            {
                                record_param_type->link -= 1;

                                record_arg->link -= 1;
                                
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

                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;
                                    
                                    return -1;
                                }
                            }

                            record_arg = record_arg2;
                        }
                    }

                    record_param_type->link -= 1;
                }

                if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (record_arg->link > 1)
                    {
                        record_arg = sy_record_copy(record_arg);
                    }
                }

                sy_record_tuple_t *tuple2 = sy_record_make_tuple(record_arg, tuple);
                if (tuple2 == ERROR)
                {
                    record_arg->link -= 1;

                    if (tuple)
                    {
                        if (sy_record_tuple_destroy(tuple) < 0)
                        {
                            record_arg->link -= 1;
                            return -1;
                        }
                    }

                    record_arg->link -= 1;
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

            sy_entry_t *entry = sy_strip_input_push(strip, scope, item1, parameter->key, record_arg);
            if (entry == ERROR)
            {
                record_arg->link -= 1;
                return -1;
            }

            item1 = item1->next;
        }
        else
        if ((parameter->flag & SYNTAX_MODIFIER_KWARG) == SYNTAX_MODIFIER_KWARG)
        {
            sy_error_type_by_node(base, "'%s' mismatch: '%s' and '%s'", 
                "argument", sy_record_type_as_string(arg), "kwarg");
            return -1;
        }
        else
        {
            sy_record_t *record_arg = sy_record_copy(arg);
            if (record_arg == ERROR)
            {
                return -1;
            }

            if (parameter->type)
            {
                sy_record_t *record_param_type = sy_execute_expression(parameter->type, strip, applicant, NULL);
                if (record_param_type == ERROR)
                {
                    record_arg->link -= 1;
                    return -1;
                }

                if (record_param_type->kind != RECORD_KIND_TYPE)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                    sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                        basic1->value, sy_record_type_as_string(record_param_type));
                    
                    record_param_type->link -= 1;

                    record_arg->link -= 1;

                    return -1;
                }

                int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                if (r1 < 0)
                {
                    record_param_type->link -= 1;

                    record_arg->link -= 1;
                    
                    return -1;
                }
                else
                if (r1 == 0)
                {
                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                    {
                        sy_error_type_by_node(base, "'%s' mismatch: '%s' and '%s'", 
                            "argument", sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));
                        
                        record_param_type->link -= 1;

                        record_arg->link -= 1;
                        
                        return -1;
                    }
                    else
                    {
                        if (record_arg->link > 1)
                        {
                            record_arg = sy_record_copy(record_arg);
                        }

                        sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                        if (record_arg2 == ERROR)
                        {
                            record_param_type->link -= 1;

                            record_arg->link -= 1;
                            
                            return -1;
                        }
                        else
                        if (record_arg2 == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                            sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                            record_param_type->link -= 1;

                            record_arg->link -= 1;
                            
                            return -1;
                        }

                        record_arg = record_arg2;
                    }
                }

                record_param_type->link -= 1;
            }
            
            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
            {
                if (record_arg->link > 1)
                {
                    record_arg = sy_record_copy(record_arg);
                }
            }

            sy_entry_t *entry = sy_strip_input_push(strip, scope, item1, parameter->key, record_arg);
            if (entry == ERROR)
            {
                return -1;
            }

            item1 = item1->next;
        }
        
        for (;item1 != NULL;item1 = item1->next)
        {
            sy_node_parameter_t *parameter = (sy_node_parameter_t *)item1->value;
            sy_entry_t *entry = sy_strip_input_find(strip, scope, parameter->key);
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
                            record_arg->link -= 1;
                            return -1;
                        }

                        if (record_param_type->kind != RECORD_KIND_TYPE)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                            sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                basic1->value, sy_record_type_as_string(record_param_type));
                            
                            record_param_type->link -= 1;

                            record_arg->link -= 1;

                            return -1;
                        }

                        int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                        if (r1 < 0)
                        {
                            record_param_type->link -= 1;

                            record_arg->link -= 1;
                            
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
                                
                                record_param_type->link -= 1;

                                record_arg->link -= 1;
                                
                                return -1;
                            }
                            else
                            {
                                if (record_arg->link > 1)
                                {
                                    record_arg = sy_record_copy(record_arg);
                                }

                                sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                if (record_arg2 == ERROR)
                                {
                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;
                                    
                                    return -1;
                                }
                                else
                                if (record_arg2 == NULL)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                    sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                        basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;
                                    
                                    return -1;
                                }

                                record_arg = record_arg2;
                            }
                        }

                        record_param_type->link -= 1;
                    }

                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (record_arg->link > 1)
                        {
                            record_arg = sy_record_copy(record_arg);
                        }
                    }

                    sy_entry_t *entry2 = sy_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                    if (entry2 == ERROR)
                    {
                        record_arg->link -= 1;
                        return -1;
                    }
                }
                else
                {
                    if (scope->kind == NODE_KIND_FUN)
                    {
                        sy_node_fun_t *fun1 = (sy_node_fun_t *)scope->value;
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
    else
    {
        if (scope->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)scope->value;
            sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
            sy_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", basic1->value, 0, 1);
            return -1;
        }
        else
        {
            sy_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "lambda", 0, 1);
            return -1;
        }
    }
    return 0;
}

int32_t
sy_execute_parameters_substitute(sy_node_t *base, sy_node_t *scope, sy_strip_t *strip, sy_node_t *parameters, sy_node_t *arguments, sy_node_t *applicant)
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

            if (scope->kind == NODE_KIND_FUN)
            {
                sy_node_fun_t *fun1 = (sy_node_fun_t *)scope->value;
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
                        if (scope->kind == NODE_KIND_FUN)
                        {
                            sy_node_fun_t *fun1 = (sy_node_fun_t *)scope->value;
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

                        if (scope->kind == NODE_KIND_FUN)
                        {
                            sy_node_fun_t *fun1 = (sy_node_fun_t *)scope->value;
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
                        sy_record_object_t *object = NULL, *top = NULL;

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
                                    record_arg->link -= 1;
                                    return -1;
                                }

                                if (record_param_type->kind != RECORD_KIND_TYPE)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                    sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                        basic1->value, sy_record_type_as_string(record_param_type));
                                    
                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;

                                    return -1;
                                }

                                int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                                if (r1 < 0)
                                {
                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;
                                    
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
                                            
                                            record_param_type->link -= 1;

                                            record_arg->link -= 1;
                                            
                                            return -1;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (record_arg->link > 1)
                                        {
                                            record_arg = sy_record_copy(record_arg);
                                        }

                                        sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                        if (record_arg2 == ERROR)
                                        {
                                            record_param_type->link -= 1;

                                            record_arg->link -= 1;
                                            
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

                                                record_param_type->link -= 1;

                                                record_arg->link -= 1;
                                                
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

                                record_param_type->link -= 1;
                            }

                            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                            {
                                if (record_arg->link > 1)
                                {
                                    record_arg = sy_record_copy(record_arg);
                                }
                            }

                            sy_record_object_t *object2 = sy_record_make_object(argument->key, record_arg, NULL);
                            if (object2 == ERROR)
                            {
                                record_arg->link -= 1;

                                if (object)
                                {
                                    if (sy_record_object_destroy(object) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                return -1;
                            }

                            if (object == NULL)
                            {
                                object = top = object2;
                            }
                            else
                            {
                                object->next = object2;
                                object = object->next;
                            }
                        }

                        sy_record_t *record_arg = sy_record_create(RECORD_KIND_OBJECT, top);
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

                        sy_entry_t *entry = sy_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                        if (entry == ERROR)
                        {
                            record_arg->link -= 1;
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
                                        record_arg->link -= 1;
                                        return -1;
                                    }

                                    if (record_param_type->kind != RECORD_KIND_TYPE)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                        sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                            basic1->value, sy_record_type_as_string(record_param_type));
                                        
                                        record_param_type->link -= 1;

                                        record_arg->link -= 1;

                                        return -1;
                                    }

                                    int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                                    if (r1 < 0)
                                    {
                                        record_param_type->link -= 1;

                                        record_arg->link -= 1;
                                        
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
                                            
                                            record_param_type->link -= 1;

                                            record_arg->link -= 1;
                                            
                                            return -1;
                                        }
                                        else
                                        {
                                            if (record_arg->link > 1)
                                            {
                                                record_arg = sy_record_copy(record_arg);
                                            }

                                            sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                            if (record_arg2 == ERROR)
                                            {
                                                record_param_type->link -= 1;

                                                record_arg->link -= 1;
                                                
                                                return -1;
                                            }
                                            else
                                            if (record_arg2 == NULL)
                                            {
                                                sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                                sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                                    basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                                                record_param_type->link -= 1;

                                                record_arg->link -= 1;
                                                
                                                return -1;
                                            }

                                            record_arg = record_arg2;
                                        }
                                    }

                                    record_param_type->link -= 1;
                                }

                                if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                                {
                                    if (record_arg->link > 1)
                                    {
                                        record_arg = sy_record_copy(record_arg);
                                    }
                                }

                                sy_entry_t *entry = sy_strip_input_push(strip, scope, item3, parameter->key, record_arg);
                                if (entry == ERROR)
                                {
                                    record_arg->link -= 1;
                                    return -1;
                                }

                                break;
                            }
                        }

                        if (found == 0)
                        {
                            if (scope->kind == NODE_KIND_FUN)
                            {
                                sy_node_fun_t *fun1 = (sy_node_fun_t *)scope->value;
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
                        sy_record_tuple_t *tuple = NULL, *top = NULL;

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
                                    record_arg->link -= 1;
                                    return -1;
                                }

                                if (record_param_type->kind != RECORD_KIND_TYPE)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                    sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                        basic1->value, sy_record_type_as_string(record_param_type));
                                    
                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;

                                    return -1;
                                }

                                int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                                if (r1 < 0)
                                {
                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;
                                    
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
                                            
                                            record_param_type->link -= 1;

                                            record_arg->link -= 1;
                                            
                                            return -1;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (record_arg->link > 1)
                                        {
                                            record_arg = sy_record_copy(record_arg);
                                        }

                                        sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                        if (record_arg2 == ERROR)
                                        {
                                            record_param_type->link -= 1;

                                            record_arg->link -= 1;
                                            
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

                                                record_param_type->link -= 1;

                                                record_arg->link -= 1;
                                                
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

                                record_param_type->link -= 1;
                            }

                            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                            {
                                if (record_arg->link > 1)
                                {
                                    record_arg = sy_record_copy(record_arg);
                                }
                            }

                            sy_record_tuple_t *tuple2 = sy_record_make_tuple(record_arg, NULL);
                            if (tuple2 == ERROR)
                            {
                                record_arg->link -= 1;

                                if (tuple)
                                {
                                    if (sy_record_tuple_destroy(tuple) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                return -1;
                            }

                            if (tuple == NULL)
                            {
                                tuple = top = tuple2;
                            }
                            else
                            {
                                tuple->next = tuple2;
                                tuple = tuple->next;
                            }
                        }

                        sy_record_t *record_arg = sy_record_create(RECORD_KIND_TUPLE, top);
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

                        sy_entry_t *entry = sy_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                        if (entry == ERROR)
                        {
                            record_arg->link -= 1;
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

                        record_arg->link -= 1;
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
                                record_arg->link -= 1;
                                return -1;
                            }

                            if (record_param_type->kind != RECORD_KIND_TYPE)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                    basic1->value, sy_record_type_as_string(record_param_type));
                                
                                record_param_type->link -= 1;

                                record_arg->link -= 1;

                                return -1;
                            }

                            int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                            if (r1 < 0)
                            {
                                record_param_type->link -= 1;

                                record_arg->link -= 1;
                                
                                return -1;
                            }
                            else
                            if (r1 == 0)
                            {
                                if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                {
                                    sy_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'", 
                                        "argument", sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));
                                    
                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;
                                    
                                    return -1;
                                }
                                else
                                {
                                    if (record_arg->link > 1)
                                    {
                                        record_arg = sy_record_copy(record_arg);
                                    }

                                    sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                    if (record_arg2 == ERROR)
                                    {
                                        record_param_type->link -= 1;

                                        record_arg->link -= 1;
                                        
                                        return -1;
                                    }
                                    else
                                    if (record_arg2 == NULL)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                        sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                            basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                                        record_param_type->link -= 1;

                                        record_arg->link -= 1;
                                        
                                        return -1;
                                    }

                                    record_arg = record_arg2;
                                }
                            }

                            record_param_type->link -= 1;
                        }
                        
                        if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                        {
                            if (record_arg->link > 1)
                            {
                                record_arg = sy_record_copy(record_arg);
                            }
                        }

                        sy_entry_t *entry = sy_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                        if (entry == ERROR)
                        {
                            record_arg->link -= 1;
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
            sy_entry_t *entry = sy_strip_input_find(strip, scope, parameter->key);
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
                            record_arg->link -= 1;
                            return -1;
                        }

                        if (record_param_type->kind != RECORD_KIND_TYPE)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                            sy_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'", 
                                basic1->value, sy_record_type_as_string(record_param_type));
                            
                            record_param_type->link -= 1;

                            record_arg->link -= 1;

                            return -1;
                        }

                        int32_t r1 = sy_execute_value_check_by_type(record_arg, record_param_type, strip, applicant);
                        if (r1 < 0)
                        {
                            record_param_type->link -= 1;

                            record_arg->link -= 1;
                            
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
                                
                                record_param_type->link -= 1;

                                record_arg->link -= 1;
                                
                                return -1;
                            }
                            else
                            {
                                if (record_arg->link > 1)
                                {
                                    record_arg = sy_record_copy(record_arg);
                                }

                                sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_arg, record_param_type, strip, applicant);
                                if (record_arg2 == ERROR)
                                {
                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;
                                    
                                    return -1;
                                }
                                else
                                if (record_arg2 == NULL)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter->key->value;
                                    sy_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'", 
                                        basic1->value, sy_record_type_as_string(record_arg), sy_record_type_as_string(record_param_type));

                                    record_param_type->link -= 1;

                                    record_arg->link -= 1;
                                    
                                    return -1;
                                }

                                record_arg = record_arg2;
                            }
                        }

                        record_param_type->link -= 1;
                    }

                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (record_arg->link > 1)
                        {
                            record_arg = sy_record_copy(record_arg);
                        }
                    }

                    sy_entry_t *entry2 = sy_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                    if (entry2 == ERROR)
                    {
                        record_arg->link -= 1;
                        return -1;
                    }
                }
                else
                {
                    if (scope->kind == NODE_KIND_FUN)
                    {
                        sy_node_fun_t *fun1 = (sy_node_fun_t *)scope->value;
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
sy_execute_heritage_substitute(sy_node_t *scope, sy_strip_t *strip, sy_node_t *node, sy_node_t *applicant)
{
    sy_node_heritage_t *heritage = (sy_node_heritage_t *)node->value;

    sy_record_t *record_heritage = sy_execute_expression(heritage->type, strip, applicant, NULL);
    if (record_heritage == ERROR)
    {
        return -1;
    }

    sy_node_class_t *class1 = (sy_node_class_t *)scope->value;

    if (record_heritage->kind != RECORD_KIND_TYPE)
    {
        sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage->key->value;
        sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
        sy_error_type_by_node(heritage->key, "'%s' unexpected type as heritage '%s'", 
            basic2->value, basic1->value);

        record_heritage->link -= 1;
        return -1;
    }

    sy_record_type_t *record_type = (sy_record_type_t *)record_heritage->value;
    sy_node_t *type = record_type->type;

    if (type->kind == NODE_KIND_CLASS)
    {
        sy_strip_t *strip_new = (sy_strip_t *)record_type->value;

        sy_record_t *content = sy_execute_provide_class(strip_new, type, applicant);
        if (content == ERROR)
        {
            record_heritage->link -= 1;
            return -1;
        }

        record_heritage->link -= 1;

        content->readonly = 1;
        content->typed = 1;

        sy_entry_t *entry = sy_strip_variable_push(strip, scope, node, heritage->key, content);
        if (entry == ERROR)
        {
            content->link -= 1;
            return -1;
        }
        else
        if (entry == NULL)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage->key->value;
            sy_error_type_by_node(heritage->key, "'%s' already set", basic1->value);
            content->link -= 1;
            return -1;
        }
    }
    else
    {
        sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage->key->value;
        sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
        sy_error_type_by_node(heritage->key, "'%s' unexpected type as heritage '%s'", 
            basic2->value, basic1->value);

        record_heritage->link -= 1;
        return -1;
    }

    return 0;
}

static int32_t
sy_execute_property_substitute(sy_node_t *scope, sy_strip_t *strip, sy_node_t *node, sy_node_t *applicant)
{
    sy_node_property_t *property = (sy_node_property_t *)node->value;
    sy_record_t *record_value = NULL;
    
    if (property->value)
    {
        
        record_value = sy_execute_expression(property->value, strip, applicant, NULL);
        if (record_value == ERROR)
        {
            return -1;
        }
        
        if (property->type)
        {
            sy_record_t *record_param_type = sy_execute_expression(property->type, strip, applicant, NULL);
            if (record_param_type == ERROR)
            {
                record_value->link -= 1;
                return -1;
            }

            if (record_param_type->kind != RECORD_KIND_TYPE)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)property->key->value;
                sy_error_type_by_node(property->type, "'%s' unsupported type: '%s'", 
                    basic1->value, sy_record_type_as_string(record_param_type));
                
                record_param_type->link -= 1;

                record_value->link -= 1;

                return -1;
            }

            int32_t r1 = sy_execute_value_check_by_type(record_value, record_param_type, strip, applicant);
            if (r1 < 0)
            {
                record_param_type->link -= 1;

                record_value->link -= 1;
                
                return -1;
            }
            else
            if (r1 == 0)
            {
                if (record_value->link > 0)
                {
                    record_value = sy_record_copy(record_value);
                }

                sy_record_t *record_arg2 = sy_execute_value_casting_by_type(record_value, record_param_type, strip, applicant);
                if (record_arg2 == ERROR)
                {
                    record_param_type->link -= 1;

                    record_value->link -= 1;
                    
                    return -1;
                }
                else
                if (record_arg2 == NULL)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)property->key->value;
                    sy_error_type_by_node(property->key, "'%s' mismatch: '%s' and '%s'", 
                        basic1->value, sy_record_type_as_string(record_value), sy_record_type_as_string(record_param_type));

                    record_param_type->link -= 1;

                    record_value->link -= 1;
                    
                    return -1;
                }

                record_value = record_arg2;
            }

            record_param_type->link -= 1;
        }
    }
    else
    {
        record_value = sy_record_make_undefined();
        if (record_value == ERROR)
        {
            return -1;
        }
    }
    
    if (record_value->link > 0)
    {
        record_value = sy_record_copy(record_value);
    }

    if ((property->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
    {
        record_value->readonly = 1;
    }
    
    sy_entry_t *entry = sy_strip_variable_push(strip, scope, node, property->key, record_value);
    if (entry == ERROR)
    {
        record_value->link -= 1;
        return -1;
    }

    return 0;
}

static sy_record_t *
sy_execute_provide_class(sy_strip_t *strip, sy_node_t *node, sy_node_t *applicant)
{
    sy_node_class_t *class1 = (sy_node_class_t *)node->value;
    
    sy_strip_t *strip_class = sy_strip_copy(strip);
    if (strip_class == ERROR)
    {
        return ERROR;
    }
    
    if (class1->heritages)
    {
        sy_node_block_t *block = (sy_node_block_t *)class1->heritages->value;
        for (sy_node_t *item = block->items;item != NULL;item = item->next)
        {
            if (sy_execute_heritage_substitute(node, strip_class, item, applicant) < 0)
            {
                if (sy_strip_destroy(strip_class) < 0)
                {
                    return ERROR;
                }
            }
        }
    }
    
    for (sy_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_PROPERTY)
        {
            sy_node_property_t *property = (sy_node_property_t *)item->value;
            if ((property->flag & SYNTAX_MODIFIER_STATIC ) != SYNTAX_MODIFIER_STATIC)
            {
                if (sy_execute_property_substitute(node, strip_class, item, applicant) < 0)
                {
                    if (sy_strip_destroy(strip_class) < 0)
                    {
                        return ERROR;
                    }
                }
            }
        }
    }
    
    sy_record_t *result = sy_record_make_struct(node, strip_class);
    if (result == ERROR)
    {
        if (sy_strip_destroy(strip_class) < 0)
        {
            return ERROR;
        }
    }
    
    return result;
}

sy_record_t *
sy_execute_call_for_operator_by_one_argument(sy_node_t *base, sy_record_t *content, sy_record_t *arg, const char *operator, sy_node_t *applicant)
{    
    sy_record_struct_t *struct1 = (sy_record_struct_t *)content->value;
    sy_node_t *type = struct1->type;
    sy_strip_t *strip_class = struct1->value;

    sy_node_class_t *class1 = (sy_node_class_t *)type->value;

    for (sy_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)item->value;
            if (sy_execute_id_strcmp(fun1->key, operator) == 1)
            {
                sy_strip_t *strip_copy = sy_strip_copy(strip_class);
                if (strip_copy == ERROR)
                {
                    return ERROR;
                }

                sy_strip_t *strip_fun = sy_strip_create(strip_copy);
                if (strip_fun == ERROR)
                {
                    if (sy_strip_destroy(strip_copy) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (sy_execute_parameters_substitute_by_one_argument(base, item, strip_fun, fun1->parameters, arg, applicant) < 0)
                {
                    if (sy_strip_destroy(strip_fun) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                int32_t r1 = sy_execute_run_fun(item, strip_fun, applicant);
                if (r1 == -1)
                {
                    if (sy_strip_destroy(strip_fun) < 0)
                    {
                        return ERROR;
                    }

                    return ERROR;
                }

                if (sy_strip_destroy(strip_fun) < 0)
                {
                    return ERROR;
                }

                sy_record_t *rax = sy_thread_get_and_set_rax(NULL);
                if (rax == ERROR)
                {
                    return ERROR;
                }
                else
                if (!rax)
                {
                    rax = sy_record_make_undefined();
                }
 
                return rax;
            }
        }
    }
    
    sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;
    sy_error_type_by_node(base, "'%s' no operator %s was found", basic1->value, operator);
    
    return ERROR;
}

static sy_record_t *
sy_execute_call_for_class(sy_node_t *base, sy_node_t *arguments, sy_strip_t *strip, sy_node_t *node, sy_node_t *applicant)
{    
    sy_record_t *content = sy_execute_provide_class(strip, node, applicant);
    if (content == ERROR)
    {
        return ERROR;
    }

    sy_record_struct_t *struct1 = (sy_record_struct_t *)content->value;
    sy_node_t *type = struct1->type;
    sy_strip_t *strip_class = struct1->value;

    sy_node_class_t *class1 = (sy_node_class_t *)type->value;

    for (sy_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)item->value;
            if (sy_execute_id_strcmp(fun1->key, CONSTRUCTOR_STR) == 1)
            {
                sy_strip_t *strip_copy = sy_strip_copy(strip_class);
                if (strip_copy == ERROR)
                {
                    return ERROR;
                }

                sy_strip_t *strip_fun = sy_strip_create(strip_copy);
                if (strip_fun == ERROR)
                {
                    content->link -= 1;
                    if (sy_strip_destroy(strip_copy) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (sy_execute_parameters_substitute(base, item, strip_fun, fun1->parameters, arguments, applicant) < 0)
                {
                    if (sy_strip_destroy(strip_fun) < 0)
                    {
                        content->link -= 1;
                        return ERROR;
                    }
                    content->link -= 1;
                    return ERROR;
                }
                int32_t r1 = sy_execute_run_fun(item, strip_fun, applicant);
                if (r1 == -1)
                {
                    if (sy_strip_destroy(strip_fun) < 0)
                    {
                        content->link -= 1;
                        return ERROR;
                    }

                    content->link -= 1;

                    return ERROR;
                }

                if (sy_strip_destroy(strip_fun) < 0)
                {
                    content->link -= 1;
                    return ERROR;
                }

                sy_record_t *rax = sy_thread_get_and_set_rax(NULL);
                if (rax == ERROR)
                {
                    return ERROR;
                }
                else
                if (!rax)
                {
                    rax = content;
                }
                else
                {
                    content->link -= 1;
                }

                return rax;
            }
        }
    }

    sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;
    sy_error_type_by_node(base, "'%s' no constructor was found", basic1->value);
    content->link -= 1;
    return ERROR;
}

static sy_record_t *
sy_execute_call_for_fun(sy_node_t *base, sy_node_t *arguments, sy_strip_t *strip, sy_node_t *node, sy_node_t *applicant)
{
    sy_node_fun_t *fun1 = (sy_node_fun_t *)node->value;
    
    sy_strip_t *strip_copy = sy_strip_copy(strip);
    if (strip_copy == ERROR)
    {
        return ERROR;
    }
    
    sy_strip_t *strip_lambda = sy_strip_create(strip_copy);
    if (strip_lambda == ERROR)
    {
        if (sy_strip_destroy(strip_copy) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    if (sy_execute_parameters_substitute(base, node, strip_lambda, fun1->parameters, arguments, applicant) < 0)
    {
        if (sy_strip_destroy(strip_lambda) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }
    
    int32_t r1 = sy_execute_run_fun(node, strip_lambda, applicant);
    if (r1 == -1)
    {
        if (sy_strip_destroy(strip_lambda) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }
    
    if (sy_strip_destroy(strip_lambda) < 0)
    {
        return ERROR;
    }
    
    sy_record_t *rax = sy_thread_get_and_set_rax(NULL);
    if (rax == ERROR)
    {
        return ERROR;
    }
    else
    if (!rax)
    {
        rax = sy_record_make_undefined();
        if (rax == ERROR)
        {
            return ERROR;
        }
    }

    return rax;
}

static sy_record_t *
sy_execute_call_for_lambda(sy_node_t *base, sy_node_t *arguments, sy_strip_t *strip, sy_node_t *node, sy_node_t *applicant)
{
    sy_node_lambda_t *fun1 = (sy_node_lambda_t *)node->value;

    sy_strip_t *strip_copy = sy_strip_copy(strip);
    if (strip_copy == ERROR)
    {
        return ERROR;
    }

    sy_strip_t *strip_lambda = sy_strip_create(strip_copy);
    if (strip_lambda == ERROR)
    {
        if (sy_strip_destroy(strip_copy) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    if (sy_execute_parameters_substitute(base, node, strip_lambda, fun1->parameters, arguments, applicant) < 0)
    {
        if (sy_strip_destroy(strip_lambda) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    int32_t r1 = sy_execute_run_lambda(node, strip_lambda, applicant);
    if (r1 == -1)
    {
        if (sy_strip_destroy(strip_lambda) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    if (sy_strip_destroy(strip_lambda) < 0)
    {
        return ERROR;
    }

    sy_record_t *rax = sy_thread_get_and_set_rax(NULL);
    if (rax == ERROR)
    {
        return ERROR;
    }
    else
    if (!rax)
    {
        rax = sy_record_make_undefined();
        if (rax == ERROR)
        {
            return ERROR;
        }
    }

    return rax;
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
            sy_strip_t *strip_new = (sy_strip_t *)record_type->value;
            sy_record_t *result = sy_execute_call_for_class(node, carrier->data, strip_new, type, applicant);
            base->link -= 1;

            if (result == ERROR)
            {
                return ERROR;
            }
            return result;
        }
        else
        if (type->kind == NODE_KIND_FUN)
        {
            sy_strip_t *strip_new = (sy_strip_t *)record_type->value;
            sy_record_t *result = sy_execute_call_for_fun(node, carrier->data, strip_new, type, applicant);
 
            base->link -= 1;

            if (result == ERROR)
            {
                return ERROR;
            }
            
            return result;
        }
        else
        if (type->kind == NODE_KIND_LAMBDA)
        {
            sy_strip_t *strip_new = (sy_strip_t *)record_type->value;
            sy_record_t *result = sy_execute_call_for_lambda(node, carrier->data, strip_new, type, applicant);
            
            base->link -= 1;

            if (result == ERROR)
            {
                return ERROR;
            }
            
            return result;
        }
        else
        if (type->kind == NODE_KIND_KINT8)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_int8(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)mpz_get_si(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_int8((int8_t)mpf_get_si(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KINT16)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_int16(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)mpz_get_si(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_int16((int16_t)mpf_get_si(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;
                
                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KINT32)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_int32(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)mpz_get_si(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_int32((int32_t)mpf_get_si(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KINT64)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_int64(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)mpz_get_si(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_int64((int64_t)mpf_get_si(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KUINT8)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_uint8(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)mpz_get_ui(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_uint8((uint8_t)mpf_get_ui(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KUINT16)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_uint16(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)mpz_get_ui(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_uint16((uint16_t)mpf_get_ui(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KUINT32)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_uint32(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)mpz_get_ui(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_uint32((uint32_t)mpf_get_ui(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KUINT64)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_uint64(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)mpz_get_ui(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_uint64((uint64_t)mpf_get_ui(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KCHAR)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_char(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_char((char)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_char((char)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_char((char)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_char((char)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_char((char)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_char((char)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_char((char)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_char((char)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_char((char)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_char((char)mpz_get_si(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_char((char)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_char((char)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_char((char)mpf_get_si(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KFLOAT32)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_float32(0.0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_float32((float)mpz_get_d(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_float32((float)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_float32((float)mpf_get_d(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KFLOAT64)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_float64(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_float64((double)mpz_get_d(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_float64((double)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_float64((double)mpf_get_d(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KBIGINT)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_bigint_from_si(0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_bigint_from_si((int64_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_bigint_from_si((int64_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_bigint_from_si((int64_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_bigint_from_si((int64_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_bigint_from_ui((uint64_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_bigint_from_ui((uint64_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_bigint_from_ui((uint64_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_bigint_from_ui((uint64_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_bigint_from_si((int64_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_bigint_from_z(*(mpz_t *)record_value->value);
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_bigint_from_si((int64_t)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_bigint_from_si((int64_t)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_bigint_from_f(*(mpf_t *)record_value->value);
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        if (type->kind == NODE_KIND_KBIGFLOAT)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_d(0.0);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }

            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;
            sy_node_argument_t *argument = (sy_node_argument_t *)block->items->value;
            if (argument->value)
            {
                sy_error_type_by_node(argument->key, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }
            sy_record_t *record_value = sy_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }
            
            if (record_value->kind == RECORD_KIND_INT8)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_si((int64_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT16)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_si((int64_t)(*(int16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT32)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_si((int64_t)(*(int32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_INT64)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_si((int64_t)(*(int64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT8)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_ui((uint64_t)(*(uint8_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT16)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_ui((uint64_t)(*(uint16_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT32)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_ui((uint64_t)(*(uint32_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_UINT64)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_ui((uint64_t)(*(uint64_t *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_si((int64_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGINT)
            {
                sy_record_t *result = sy_record_make_bigint_from_z(*(mpz_t *)record_value->value);
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT32)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_d((double)(*(float *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_FLOAT64)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_d((double)(*(double *)record_value->value));
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            if (record_value->kind == RECORD_KIND_BIGFLOAT)
            {
                sy_record_t *result = sy_record_make_bigfloat_from_f(*(mpf_t *)record_value->value);
                if (result == ERROR)
                {
                    record_value->link -= 1;

                    base->link -= 1;
                    return ERROR;
                }

                record_value->link -= 1;

                base->link -= 1;

                return result;
            }
            else
            {
                sy_error_type_by_node(carrier->base, "'%s' object is not castable", 
                    sy_record_type_as_string(base));

                base->link -= 1;

                return ERROR;
            }
        }
        else
        {
            sy_error_type_by_node(carrier->base, "'%s' object is not callable", 
                sy_record_type_as_string(base));

            base->link -= 1;

            return ERROR;
        }
        
    }
    
    sy_error_type_by_node(carrier->base, "'%s' object is not callable", 
        sy_record_type_as_string(base));

    base->link -= 1;

    return ERROR;
}
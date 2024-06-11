#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#include "../../types/types.h"
#include "../../container/queue.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../../error.h"
#include "../../mutex.h"
#include "../../config.h"
#include "../../interpreter.h"
#include "../../thread.h"
#include "../../memory.h"
#include "../../scanner/scanner.h"
#include "../../parser/syntax/syntax.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "../entry.h"
#include "execute.h"

static sy_record_t *
sy_execute_provide_class(sy_strip_t *strip, sy_node_t *node, sy_node_t *applicant);

int32_t 
sy_execute_parameters_check_by_one_argument(sy_strip_t *strip, sy_node_t *parameters, sy_record_t *arg, sy_node_t *applicant)
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
                                record_param_type->link -= 1;
                                record_arg->link -= 1;

                                return 0;
                            }
                        }
                        else
                        {
                            if (record_arg->link > 1)
                            {
                                sy_record_t *record_copy = sy_record_copy(record_arg);
                                record_arg->link -= 1;
                                record_arg = record_copy;
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
                                    record_param_type->link -= 1;
                                    record_arg->link -= 1;
                                    
                                    return 0;
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
                        sy_record_t *record_copy = sy_record_copy(record_arg);
                        record_arg->link -= 1;
                        record_arg = record_copy;
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

            if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
            {
                record_arg->readonly = 1;
            }

            if (parameter->type)
            {
                record_arg->typed = 1;
            }

            record_arg->link -= 1;

            item1 = item1->next;
        }
        else
        if ((parameter->flag & SYNTAX_MODIFIER_KWARG) == SYNTAX_MODIFIER_KWARG)
        {
            return 0;
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
                        record_param_type->link -= 1;
                        record_arg->link -= 1;
                        
                        return 0;
                    }
                    else
                    {
                        if (record_arg->link > 1)
                        {
                            sy_record_t *record_copy = sy_record_copy(record_arg);
                            record_arg->link -= 1;
                            record_arg = record_copy;
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
                            record_param_type->link -= 1;
                            record_arg->link -= 1;
                            
                            return 0;
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
                    sy_record_t *record_copy = sy_record_copy(record_arg);
                    record_arg->link -= 1;
                    record_arg = record_copy;
                }
            }

            if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
            {
                record_arg->readonly = 1;
            }

            if (parameter->type)
            {
                record_arg->typed = 1;
            }

            record_arg->link -= 1;

            item1 = item1->next;
        }
        
        for (;item1 != NULL;item1 = item1->next)
        {
            sy_node_parameter_t *parameter = (sy_node_parameter_t *)item1->value;
            if (!parameter->value)
            {
                return 0;
            }
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

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
                                sy_record_t *record_copy = sy_record_copy(record_arg);
                                record_arg->link -= 1;
                                record_arg = record_copy;
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
                        sy_record_t *record_copy = sy_record_copy(record_arg);
                        record_arg->link -= 1;
                        record_arg = record_copy;
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

            if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
            {
                record_arg->readonly = 1;
            }

            if (parameter->type)
            {
                record_arg->typed = 1;
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
                            sy_record_t *record_copy = sy_record_copy(record_arg);
                            record_arg->link -= 1;
                            record_arg = record_copy;
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
                    sy_record_t *record_copy = sy_record_copy(record_arg);
                    record_arg->link -= 1;
                    record_arg = record_copy;
                }
            }

            if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
            {
                record_arg->readonly = 1;
            }

            if (parameter->type)
            {
                record_arg->typed = 1;
            }

            sy_entry_t *entry = sy_strip_input_push(strip, scope, item1, parameter->key, record_arg);
            if (entry == ERROR)
            {
                record_arg->link -= 1;
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
                                    sy_record_t *record_copy = sy_record_copy(record_arg);
                                    record_arg->link -= 1;
                                    record_arg = record_copy;
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
                            sy_record_t *record_copy = sy_record_copy(record_arg);
                            record_arg->link -= 1;
                            record_arg = record_copy;
                        }
                    }

                    if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                    {
                        record_arg->readonly = 1;
                    }

                    if (parameter->type)
                    {
                        record_arg->typed = 1;
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
                                            sy_record_t *record_copy = sy_record_copy(record_arg);
                                            record_arg->link -= 1;
                                            record_arg = record_copy;
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
                                    sy_record_t *record_copy = sy_record_copy(record_arg);
                                    record_arg->link -= 1;
                                    record_arg = record_copy;
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

                        if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                        {
                            record_arg->readonly = 1;
                        }

                        if (parameter->type)
                        {
                            record_arg->typed = 1;
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
                                                sy_record_t *record_copy = sy_record_copy(record_arg);
                                                record_arg->link -= 1;
                                                record_arg = record_copy;
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
                                        sy_record_t *record_copy = sy_record_copy(record_arg);
                                        record_arg->link -= 1;
                                        record_arg = record_copy;
                                    }
                                }

                                if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                                {
                                    record_arg->readonly = 1;
                                }

                                if (parameter->type)
                                {
                                    record_arg->typed = 1;
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
                                            sy_record_t *record_copy = sy_record_copy(record_arg);
                                            record_arg->link -= 1;
                                            record_arg = record_copy;
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
                                    sy_record_t *record_copy = sy_record_copy(record_arg);
                                    record_arg->link -= 1;
                                    record_arg = record_copy;
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

                        if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                        {
                            record_arg->readonly = 1;
                        }

                        if (parameter->type)
                        {
                            record_arg->typed = 1;
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
                                        sy_record_t *record_copy = sy_record_copy(record_arg);
                                        record_arg->link -= 1;
                                        record_arg = record_copy;
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
                                sy_record_t *record_copy = sy_record_copy(record_arg);
                                record_arg->link -= 1;
                                record_arg = record_copy;
                            }
                        }

                        if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                        {
                            record_arg->readonly = 1;
                        }

                        if (parameter->type)
                        {
                            record_arg->typed = 1;
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
                                    sy_record_t *record_copy = sy_record_copy(record_arg);
                                    record_arg->link -= 1;
                                    record_arg = record_copy;
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
                            sy_record_t *record_copy = sy_record_copy(record_arg);
                            record_arg->link -= 1;
                            record_arg = record_copy;
                        }
                    }

                    if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                    {
                        record_arg->readonly = 1;
                    }

                    if (parameter->type)
                    {
                        record_arg->typed = 1;
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
        sy_record_t *record_copy = sy_record_copy(record_value);
        record_value->link -= 1;
        record_value = record_copy;
    }

    if ((property->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
    {
        record_value->readonly = 1;
    }

    if (property->type)
    {
        record_value->typed = 1;
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

char *
record_to_string(sy_record_t *record, char *previous_buf)
{
    if (record->kind == RECORD_KIND_CHAR)
    {
        char str[50];
        snprintf(str, sizeof(str), "%c", (char)(*(int8_t *)record->value));
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else
    if (record->kind == RECORD_KIND_STRING)
    {
        char *str = (char *)record->value;
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else
    if (record->kind == RECORD_KIND_INT)
    {
        char *str = mpz_get_str(NULL, 10, *(mpz_t *)record->value);
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        free(str);
        return result;
    }
    else
    if (record->kind == RECORD_KIND_FLOAT)
    {
        size_t buf_size = 64;
        char *str = (char *)sy_memory_calloc(buf_size, sizeof(char));

        if (!str) {
            sy_error_no_memory();
            return ERROR;
        }

        size_t required_size = gmp_snprintf(str, buf_size, "%s%.Ff", previous_buf, (*(mpf_t *)record->value));
        if (required_size >= buf_size) {
            buf_size = required_size + 1;
            str = (char *)sy_memory_realloc(str, buf_size);

            if (!str) {
                sy_error_no_memory();
                return ERROR;
            }
            gmp_snprintf(str, buf_size, "%s%.Ff", previous_buf, (*(mpf_t *)record->value));
        }

        return str;
    }
    else
    if (record->kind == RECORD_KIND_OBJECT)
    {
        size_t length = strlen(previous_buf) + 1;
        char *str = sy_memory_calloc(length + 1, sizeof(char));
        if (str == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(str, length + 1, "%s%c", previous_buf, '{');
        for (sy_record_object_t *item = (sy_record_object_t *)record->value;item != NULL;item = item->next)
        {
            sy_node_basic_t *basic = (sy_node_basic_t *)item->key->value;
            length = strlen(str) + strlen(basic->value) + 1;
            char *result = sy_memory_calloc(length + 1, sizeof(char));
            if (result == NULL)
            {
                sy_error_no_memory();
                return ERROR;
            }
            snprintf(result, length + 1, "%s%s:", str, basic->value);
            sy_memory_free(str);
            str = result;

            result = record_to_string(item->value, str);
            if (result == ERROR)
            {
                sy_memory_free(str);
                return ERROR;
            }
            sy_memory_free(str);
            str = result;

            if (item->next)
            {
                length = strlen(str) + 1;
                char *result = sy_memory_calloc(length + 1, sizeof(char));
                if (result == NULL)
                {
                    sy_error_no_memory();
                    return ERROR;
                }
                snprintf(result, length + 1, "%s,", str);
                sy_memory_free(str);
                str = result;
            }
        }
        length = strlen(str) + 1;
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            sy_memory_free(str);
            return ERROR;
        }
        snprintf(result, length + 1, "%s%c", str, '}');
        sy_memory_free(str);
        return result;
    }
    else
    if (record->kind == RECORD_KIND_TUPLE)
    {
        size_t length = strlen(previous_buf) + 1;
        char *str = sy_memory_calloc(length + 1, sizeof(char));
        if (str == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(str, length + 1, "%s%c", previous_buf, '[');

        for (sy_record_tuple_t *item = (sy_record_tuple_t *)record->value;item != NULL;item = item->next)
        {
            char *result = record_to_string(item->value, str);
            if (result == ERROR)
            {
                sy_memory_free(str);
                return ERROR;
            }
            sy_memory_free(str);
            str = result;

            if (item->next)
            {
                length = strlen(str) + 1;
                char *result = sy_memory_calloc(length + 1, sizeof(char));
                if (result == NULL)
                {
                    sy_error_no_memory();
                    return ERROR;
                }
                snprintf(result, length + 1, "%s,", str);
                sy_memory_free(str);
                str = result;
            }
        }

        length = strlen(str) + 1;
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            sy_memory_free(str);
            return ERROR;
        }
        snprintf(result, length + 1, "%s%c", str, ']');
        sy_memory_free(str);
        return result;
    }
    else
    if (record->kind == RECORD_KIND_UNDEFINED)
    {
        char *str = "undefined";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else
    if (record->kind == RECORD_KIND_NAN)
    {
        char *str = "nan";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else
    if (record->kind == RECORD_KIND_NULL)
    {
        char *str = "null";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else
    if (record->kind == RECORD_KIND_TYPE)
    {
        char *str = "<type>";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else
    if (record->kind == RECORD_KIND_STRUCT)
    {
        sy_record_struct_t *struct1 = (sy_record_struct_t *)record->value;
        sy_node_t *type = struct1->type;
        sy_strip_t *strip_class = struct1->value;

        sy_node_class_t *class1 = (sy_node_class_t *)type->value;

        size_t length = strlen(previous_buf) + 1;
        char *str = sy_memory_calloc(length + 1, sizeof(char));
        if (str == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(str, length + 1, "%s%c", previous_buf, '{');
        uint64_t i = 0;
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

                if (i > 0)
                {
                    length = strlen(str) + 1;
                    char *result = sy_memory_calloc(length + 1, sizeof(char));
                    if (result == NULL)
                    {
                        sy_error_no_memory();
                        return ERROR;
                    }
                    snprintf(result, length + 1, "%s,", str);
                    sy_memory_free(str);
                    str = result;
                }

                sy_node_basic_t *basic = (sy_node_basic_t *)property->key->value;
                length = strlen(str) + strlen(basic->value) + 1;
                char *result = sy_memory_calloc(length + 1, sizeof(char));
                if (result == NULL)
                {
                    sy_error_no_memory();
                    return ERROR;
                }
                snprintf(result, length + 1, "%s%s:", str, basic->value);
                sy_memory_free(str);
                str = result;


                sy_entry_t *entry = sy_strip_variable_find(strip_class, type, property->key);
                if (entry == ERROR)
                {
                    sy_memory_free(str);
                    return ERROR;
                }
                if (entry == NULL)
                {
                    sy_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    sy_memory_free(str);
                    return ERROR;
                }

                result = record_to_string(entry->value, str);

                entry->value->link -= 1;

                if (result == ERROR)
                {
                    sy_memory_free(str);
                    return ERROR;
                }
                sy_memory_free(str);
                str = result;
                i += 1;
            }
        }
        
        if (class1->heritages)
        {
            sy_node_block_t *block = (sy_node_block_t *)class1->heritages->value;
            for (sy_node_t *item = block->items; item != NULL; item = item->next)
            {
                sy_node_heritage_t *heritage = (sy_node_heritage_t *)item->value;
                if (i > 0)
                {
                    length = strlen(str) + 1;
                    char *result = sy_memory_calloc(length + 1, sizeof(char));
                    if (result == NULL)
                    {
                        sy_error_no_memory();
                        return ERROR;
                    }
                    snprintf(result, length + 1, "%s,", str);
                    sy_memory_free(str);
                    str = result;
                }

                sy_node_basic_t *basic = (sy_node_basic_t *)heritage->key->value;
                length = strlen(str) + strlen(basic->value) + 1;
                char *result = sy_memory_calloc(length + 1, sizeof(char));
                if (result == NULL)
                {
                    sy_error_no_memory();
                    return ERROR;
                }
                snprintf(result, length + 1, "%s%s:", str, basic->value);
                sy_memory_free(str);
                str = result;

                sy_entry_t *entry = sy_strip_variable_find(strip_class, type, heritage->key);
                if (entry == ERROR)
                {
                    sy_memory_free(str);
                    return ERROR;
                }
                if (entry == NULL)
                {
                    sy_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    sy_memory_free(str);
                    return ERROR;
                }

                result = record_to_string(entry->value, str);

                entry->value->link -= 1;

                if (result == ERROR)
                {
                    sy_memory_free(str);
                    return ERROR;
                }
                sy_memory_free(str);
                str = result;
                i += 1;
            }
        }

        length = strlen(str) + 1;
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            sy_memory_free(str);
            return ERROR;
        }
        snprintf(result, length + 1, "%s%c", str, '}');
        sy_memory_free(str);
        return result;
    }
    else
    {
        char *str = "";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
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
            if (record_value->kind == RECORD_KIND_INT)
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
            if (record_value->kind == RECORD_KIND_FLOAT)
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
        if (type->kind == NODE_KIND_KINT)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_int_from_si(0);
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

            if (record_value->kind == RECORD_KIND_CHAR)
            {
                sy_record_t *result = sy_record_make_int_from_si((int64_t)(*(char *)record_value->value));
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
            if (record_value->kind == RECORD_KIND_INT)
            {
                sy_record_t *result = sy_record_make_int_from_z(*(mpz_t *)record_value->value);
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
            if (record_value->kind == RECORD_KIND_FLOAT)
            {
                sy_record_t *result = sy_record_make_int_from_f(*(mpf_t *)record_value->value);
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
        if (type->kind == NODE_KIND_KFLOAT)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_float_from_d(0.0);
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
            
            if (record_value->kind == RECORD_KIND_INT)
            {
                sy_record_t *result = sy_record_make_float_from_si((int64_t)(*(int8_t *)record_value->value));
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
                sy_record_t *result = sy_record_make_float_from_si((int64_t)(*(char *)record_value->value));
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
            if (record_value->kind == RECORD_KIND_INT)
            {
                sy_record_t *result = sy_record_make_int_from_z(*(mpz_t *)record_value->value);
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
            if (record_value->kind == RECORD_KIND_FLOAT)
            {
                sy_record_t *result = sy_record_make_float_from_f(*(mpf_t *)record_value->value);
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
        if (type->kind == NODE_KIND_KSTRING)
        {
            if (!carrier->data)
            {
                sy_record_t *result = sy_record_make_string("");
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

            char *str = record_to_string(record_value, "");
            printf("%s\n", str);
            sy_record_t *result = sy_record_make_string(str);
            sy_memory_free(str);
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
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
#include "../entry.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"


sy_record_t *
sy_execute_selection(sy_node_t *base, sy_node_t *name, sy_node_t *origin_class, sy_strip_t *strip)
{
    if (base->kind == NODE_KIND_CATCH)
    {
        sy_node_catch_t *catch1 = (sy_node_catch_t *)base->value;

        sy_node_t *node1 = catch1->parameters;
        sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

        for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item1->value;
                if (sy_execute_id_cmp(parameter1->key, name) == 1)
                {
                    assert(strip != NULL);
                    sy_entry_t *entry = sy_strip_variable_find(strip, parameter1->key);
                    if (entry == ERROR)
                    {
                        return ERROR;
                    }
                    else
                    if (entry == NULL)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter1->key;
                        sy_error_runtime_by_node(parameter1->key, "'%s' is not initialized", basic1->value);
                        return ERROR;
                    }
                    return entry->value;
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_FOR)
    {
        sy_node_for_t *for1 = (sy_node_for_t *)base->value;

        for (sy_node_t *item1 = for1->initializer;item1 != NULL;item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_VAR)
            {
                sy_node_var_t *var1 = (sy_node_var_t *)item1->value;
                if (var1->key->kind == NODE_KIND_ID)
                {
                    if (sy_execute_id_cmp(var1->key, name) == 1)
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, var1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key;
                            sy_error_runtime_by_node(var1->key, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                }
                else
                {
                    sy_node_t *node1 = var1->key;
                    sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

                    for (sy_node_t *item2 = block1->items;item2 != NULL;item2 = item2->next)
                    {
                        if (item2->kind == NODE_KIND_ENTITY)
                        {
                            sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                            if (sy_execute_id_cmp(entity1->key, name) == 1)
                            {
                                assert(strip != NULL);
                                sy_entry_t *entry = sy_strip_variable_find(strip, entity1->key);
                                if (entry == ERROR)
                                {
                                    return ERROR;
                                }
                                else
                                if (entry == NULL)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key;
                                    sy_error_runtime_by_node(entity1->key, "'%s' is not initialized", basic1->value);
                                    return ERROR;
                                }
                                return entry->value;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_BODY)
    {
        sy_node_block_t *block1 = (sy_node_block_t *)base->value;

        for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_FOR)
            {
                sy_node_for_t *for1 = (sy_node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (sy_execute_id_cmp(for1->key, name) == 1)
                    {
                        return sy_record_make_type(item1, NULL);
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                sy_node_var_t *var1 = (sy_node_var_t *)item1->value;
                if (var1->key->kind == NODE_KIND_ID)
                {
                    if (sy_execute_id_cmp(var1->key, name) == 1)
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, var1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key;
                            sy_error_runtime_by_node(var1->key, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                }
                else
                {
                    sy_node_t *node2 = var1->key;
                    sy_node_block_t *block2 = (sy_node_block_t *)node2->value;

                    for (sy_node_t *item2 = block2->items;item2 != NULL;item2 = item2->next)
                    {
                        if (item2->kind == NODE_KIND_ENTITY)
                        {
                            sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                            if (sy_execute_id_cmp(entity1->key, name) == 1)
                            {
                                assert(strip != NULL);
                                sy_entry_t *entry = sy_strip_variable_find(strip, entity1->key);
                                if (entry == ERROR)
                                {
                                    return ERROR;
                                }
                                else
                                if (entry == NULL)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key;
                                    sy_error_runtime_by_node(entity1->key, "'%s' is not initialized", basic1->value);
                                    return ERROR;
                                }
                                return entry->value;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_LAMBDA)
    {
        sy_node_lambda_t *fun1 = (sy_node_lambda_t *)base->value;

        if (fun1->generics != NULL)
        {
            sy_node_t *node1 = fun1->generics;
            sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

            for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;
                    if (sy_execute_id_cmp(generic1->key, name) == 1)
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, generic1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                            sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                }
            }
        }

        if (fun1->parameters != NULL)
        {
            sy_node_t *node1 = fun1->parameters;
            sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

            for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item1->value;
                    if (sy_execute_id_cmp(parameter1->key, name) == 1)
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, parameter1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter1->key;
                            sy_error_runtime_by_node(parameter1->key, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_FUN)
    {
        sy_node_fun_t *fun1 = (sy_node_fun_t *)base->value;

        if (fun1->generics != NULL)
        {
            sy_node_t *node1 = fun1->generics;
            sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

            for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;
                    if (sy_execute_id_cmp(generic1->key, name) == 1)
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, generic1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                            sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                }
            }
        }

        if (fun1->parameters != NULL)
        {
            sy_node_t *node1 = fun1->parameters;
            sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

            for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item1->value;
                    if (sy_execute_id_cmp(parameter1->key, name) == 1)
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, parameter1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter1->key;
                            sy_error_runtime_by_node(parameter1->key, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_PACKAGE)
    {
        sy_node_package_t *package1 = (sy_node_package_t *)base->value;
        if (package1->generics != NULL)
        {
            sy_node_t *node1 = package1->generics;
            sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

            for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;
                    if (sy_execute_id_cmp(generic1->key, name) == 1)
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, generic1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                            sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_CLASS)
    {
        sy_node_class_t *class1 = (sy_node_class_t *)base->value;

        if (class1->generics != NULL)
        {
            sy_node_t *node1 = class1->generics;
            sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

            for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;
                    if (sy_execute_id_cmp(generic1->key, name) == 1)
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, generic1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key;
                            sy_error_runtime_by_node(generic1->key, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                }
            }
        }

        if (class1->heritages != NULL)
        {
            sy_node_t *node1 = class1->heritages;
            sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

            for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_HERITAGE)
                {
                    sy_node_heritage_t *heritage1 = (sy_node_heritage_t *)item1->value;
                    if (sy_execute_id_cmp(heritage1->key, name) == 1)
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, heritage1->key);
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
                        return entry->value;
                    }
                }
            }
        }

        sy_node_t *node1 = class1->block;
        sy_node_block_t *block1 = (sy_node_block_t *)node1->value;

        for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_CLASS)
            {
                sy_node_class_t *class2 = (sy_node_class_t *)item1->value;
                if (sy_execute_id_cmp(class2->key, name) == 1)
                {
                    if (origin_class != NULL)
                    {
                        if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            sy_error_type_by_node(name, "private access");
                            return ERROR;
                        }
                    }
                    
                    return sy_record_make_type(item1, NULL);
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_FUN)
            {
                sy_node_fun_t *fun1 = (sy_node_fun_t *)item1->value;
                if (sy_execute_id_cmp(fun1->key, name) == 1)
                {
                    if (origin_class != NULL)
                    {
                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            sy_error_type_by_node(name, "private access");
                            return ERROR;
                        }
                    }

                    return sy_record_make_type(item1, NULL);  
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                sy_node_property_t *property1 = (sy_node_property_t *)item1->value;
                if (sy_execute_id_cmp(property1->key, name) == 1)
                {
                    if (origin_class != NULL)
                    {
                        if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            sy_error_type_by_node(name, "private access");
                            return ERROR;
                        }
                    }

                    if ((property1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                    {
                        sy_entry_t *entry = sy_symbol_table_find(base, property1->key);
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
                    else
                    {
                        assert(strip != NULL);
                        sy_entry_t *entry = sy_strip_variable_find(strip, property1->key);
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
                continue;
            }
        }
    }
    else
    if (base->kind == NODE_KIND_MODULE)
    {
        sy_node_block_t *block1 = (sy_node_block_t *)base->value;

        for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_USING)
            {
                sy_node_using_t *using1 = (sy_node_using_t *)item1->value;

                if (using1->packages != NULL)
                {
                    sy_node_t *node1 = using1->packages;
                    sy_node_block_t *block2 = (sy_node_block_t *)node1->value;

                    for (sy_node_t *item2 = block2->items;item2 != NULL;item2 = item2->next)
                    {
                        if (item2->kind == NODE_KIND_PACKAGE)
                        {
                            sy_node_package_t *package1 = (sy_node_package_t *)item2->value;
                            if (sy_execute_id_cmp(package1->key, name) == 1)
                            {
                                return sy_record_make_type(item2, NULL); 
                            }
                        }
                    }
                }

                continue;
            }
            else
            if (item1->kind == NODE_KIND_CLASS)
            {
                sy_node_class_t *class1 = (sy_node_class_t *)item1->value;

                if (sy_execute_id_cmp(class1->key, name) == 1)
                {
                    return sy_record_make_type(item1, NULL); 
                }
                
                continue;
            }
            else
            if (item1->kind == NODE_KIND_FOR)
            {
                sy_node_for_t *for1 = (sy_node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (sy_execute_id_cmp(for1->key, name) == 1)
                    {
                        return sy_record_make_type(item1, NULL);
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                sy_node_var_t *var1 = (sy_node_var_t *)item1->value;
                if (var1->key->kind == NODE_KIND_ID)
                {
                    if (sy_execute_id_cmp(var1->key, name) == 1)
                    {
                        if ((var1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                        {
                            sy_entry_t *entry = sy_symbol_table_find(base, var1->key);
                            if (entry == ERROR)
                            {
                                return ERROR;
                            }
                            else
                            if (entry == NULL)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key;
                                sy_error_runtime_by_node(var1->key, "'%s' is not initialized", basic1->value);
                                return ERROR;
                            }
                            return entry->value;
                        }
                        else
                        {
                            assert(strip != NULL);
                            sy_entry_t *entry = sy_strip_variable_find(strip, var1->key);
                            if (entry == ERROR)
                            {
                                return ERROR;
                            }
                            else
                            if (entry == NULL)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key;
                                sy_error_runtime_by_node(var1->key, "'%s' is not initialized", basic1->value);
                                return ERROR;
                            }
                            return entry->value;
                        }
                    }
                }
                else
                {
                    sy_node_t *node2 = var1->key;
                    sy_node_block_t *block2 = (sy_node_block_t *)node2->value;

                    for (sy_node_t *item2 = block2->items;item2 != NULL;item2 = item2->next)
                    {
                        if (item2->kind == NODE_KIND_ENTITY)
                        {
                            sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                            if (sy_execute_id_cmp(entity1->key, name) == 1)
                            {
                                if ((entity1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                                {
                                    sy_entry_t *entry = sy_symbol_table_find(base, entity1->key);
                                    if (entry == ERROR)
                                    {
                                        return ERROR;
                                    }
                                    else
                                    if (entry == NULL)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key;
                                        sy_error_runtime_by_node(entity1->key, "'%s' is not initialized", basic1->value);
                                        return ERROR;
                                    }
                                    return entry->value;
                                }
                                else
                                {
                                    assert(strip != NULL);
                                    sy_entry_t *entry = sy_strip_variable_find(strip, entity1->key);
                                    if (entry == ERROR)
                                    {
                                        return ERROR;
                                    }
                                    else
                                    if (entry == NULL)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key;
                                        sy_error_runtime_by_node(entity1->key, "'%s' is not initialized", basic1->value);
                                        return ERROR;
                                    }
                                    return entry->value;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (base->parent != NULL)
    {
        if ((base->kind == NODE_KIND_CLASS) && (origin_class == NULL))
        {
            return sy_execute_selection(base->parent, name, base, strip);
        }
        else
        {
            return sy_execute_selection(base->parent, name, origin_class, strip);
        }
    }

    return sy_record_make_undefined();
}


sy_record_t *
sy_execute_id(sy_node_t *node, sy_strip_t *strip)
{
    return sy_execute_selection(node->parent, node, NULL, strip);
}

sy_record_t *
sy_execute_number(sy_node_t *node, sy_strip_t *strip)
{
    sy_node_basic_t *basic1 = (sy_node_basic_t *)node->value;
    char *str = basic1->value;

    int32_t base = 10;

    if (str[0] == '0')
    {
        if (str[1] == 'x' || str[1] == 'X')
        {
            base = 16;
        }
        else if (str[1] == 'b' || str[1] == 'B')
        {
            base = 2;
        }
        else if (isdigit(str[1]))
        {
            base = 8;
        } 
    }

    sy_record_t *record = NULL;

    if (strchr(str, '.'))
    {
        mpf_t result;
        mpf_init(result);
        mpf_set_str(result, str, base);

        // float
        if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
            record = sy_record_make_float32((float)mpf_get_d(result));
        } 
        // double
        else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
            record = sy_record_make_float64(mpf_get_d(result));
        } 
        // mpf_t
        else {
            record = sy_record_make_bigfloat_from_mpf(result);
        }

        mpf_clear(result);
    }
    else
    {
        mpz_t result_mpz;
        mpz_init(result_mpz);
        mpz_set_str(result_mpz, str, base);

        // int8_t
        if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
            record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
        }
        else
        // uint8_t
        if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
            record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
        }
        else
        // int16_t
        if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
            record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
        }
        else
        // uint16_t
        if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
            record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
        }
        else
        // int32_t
        if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
            record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
        }
        else
        // uint32_t
        if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
            record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
        }
        else
        // int64_t
        if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
            record = sy_record_make_int64(mpz_get_si(result_mpz));
        }
        else
        // uint64_t
        if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
            record = sy_record_make_uint64(mpz_get_ui(result_mpz));
        }
        // mpz_t
        else {
            record = sy_record_make_bigint_from_mpz(result_mpz);
        }

        mpz_clear(result_mpz);
    }
    
    return record;
}

sy_record_t *
sy_execute_char(sy_node_t *node, sy_strip_t *strip)
{
    sy_node_basic_t *basic1 = (sy_node_basic_t *)node->value;
    char *str = basic1->value;

    return sy_record_make_char((*(char *)str));
}

sy_record_t *
sy_execute_string(sy_node_t *node, sy_strip_t *strip)
{
    sy_node_basic_t *basic1 = (sy_node_basic_t *)node->value;
    char *str = basic1->value;

    return sy_record_make_string((*(char **)str));
}

sy_record_t *
sy_execute_null(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_null();
}


sy_record_t *
sy_execute_kint8(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kint16(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kint32(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kint64(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kuint8(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kuint16(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kuint32(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kuint64(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kbigint(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kfloat32(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kfloat64(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kbigfloat(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kchar(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kstring(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_self(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_undefined();
}

sy_record_t *
sy_execute_this(sy_node_t *node, sy_strip_t *strip)
{
    return sy_record_make_undefined();
}

sy_record_t *
sy_execute_lambda(sy_node_t *node, sy_strip_t *strip)
{
	return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_parenthesis(sy_node_t *node, sy_strip_t *strip)
{
    sy_node_unary_t *unary1 = (sy_node_unary_t *)node->value;

    return sy_execute_expression(unary1->right, strip);
}


sy_record_t *
sy_execute_primary(sy_node_t *node, sy_strip_t *strip)
{
    if (node->kind == NODE_KIND_ID)
    {
        return sy_execute_id(node, strip);
    }
    else

    if (node->kind == NODE_KIND_NUMBER)
    {
        return sy_execute_number(node, strip);
    }
    else
    if (node->kind == NODE_KIND_CHAR)
    {
        return sy_execute_char(node, strip);
    }
    else
    if (node->kind == NODE_KIND_STRING)
    {
        return sy_execute_string(node, strip);
    }
    else

    if (node->kind == NODE_KIND_NULL)
    {
        return sy_execute_null(node, strip);
    }
    else

    if (node->kind == NODE_KIND_KINT8)
    {
        return sy_execute_kint8(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KINT16)
    {
        return sy_execute_kint16(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KINT32)
    {
        return sy_execute_kint32(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KINT64)
    {
        return sy_execute_kint64(node, strip);
    }
    else

    if (node->kind == NODE_KIND_KUINT8)
    {
        return sy_execute_kuint8(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KUINT16)
    {
        return sy_execute_kuint16(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KUINT32)
    {
        return sy_execute_kuint32(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KUINT64)
    {
        return sy_execute_kuint64(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KBIGINT)
    {
        return sy_execute_kbigint(node, strip);
    }
    else

    if (node->kind == NODE_KIND_KFLOAT32)
    {
        return sy_execute_kfloat32(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KFLOAT64)
    {
        return sy_execute_kfloat64(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KBIGFLOAT)
    {
        return sy_execute_kbigfloat(node, strip);
    }
    else

    if (node->kind == NODE_KIND_KCHAR)
    {
        return sy_execute_kchar(node, strip);
    }
    else
    if (node->kind == NODE_KIND_KSTRING)
    {
        return sy_execute_kstring(node, strip);
    }
    else


    if (node->kind == NODE_KIND_THIS)
    {
        return sy_execute_this(node, strip);
    }
    else
    if (node->kind == NODE_KIND_SELF)
    {
        return sy_execute_self(node, strip);
    }
    else

    if (node->kind == NODE_KIND_TUPLE)
    {
        // return semantic_tuple(node, strip);
        return sy_record_make_undefined();
    }
    else
    if (node->kind == NODE_KIND_OBJECT)
    {
        // return semantic_object(node, strip);
        return sy_record_make_undefined();
    }
    else

    if (node->kind == NODE_KIND_LAMBDA)
    {
        return sy_execute_lambda(node, strip);
    }
    else
    if (node->kind == NODE_KIND_PARENTHESIS)
    {
        return sy_execute_parenthesis(node, strip);
    }
    
    sy_error_type_by_node(node, "primary implement not support");
    return ERROR;
}
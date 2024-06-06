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
#include "../../module.h"
#include "../record.h"
#include "../entry.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"


sy_record_t *
sy_execute_selection(sy_node_t *base, sy_node_t *name, sy_strip_t *strip, sy_node_t *applicant)
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
                    sy_entry_t *entry = sy_strip_variable_find(strip, base, parameter1->key);
                    if (entry == ERROR)
                    {
                        return ERROR;
                    }
                    else
                    if (entry == NULL)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter1->key->value;
                        sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                        sy_entry_t *entry = sy_strip_variable_find(strip, base, var1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                                sy_entry_t *entry = sy_strip_variable_find(strip, base, entity1->key);
                                if (entry == ERROR)
                                {
                                    return ERROR;
                                }
                                else
                                if (entry == NULL)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;
                                    sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                        sy_entry_t *entry = sy_strip_variable_find(strip, base, var1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                                sy_entry_t *entry = sy_strip_variable_find(strip, base, entity1->key);
                                if (entry == ERROR)
                                {
                                    return ERROR;
                                }
                                else
                                if (entry == NULL)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;
                                    sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                        sy_entry_t *entry = sy_strip_variable_find(strip, base, generic1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                        sy_entry_t *entry = sy_strip_variable_find(strip, base, parameter1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                        sy_entry_t *entry = sy_strip_variable_find(strip, base, generic1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                        sy_entry_t *entry = sy_strip_variable_find(strip, base, parameter1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                        sy_entry_t *entry = sy_strip_variable_find(strip, base, generic1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                        sy_entry_t *entry = sy_strip_variable_find(strip, base, heritage1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                }
            }
        }

        for (sy_node_t *item1 = class1->block;item1 != NULL;item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_CLASS)
            {
                sy_node_class_t *class2 = (sy_node_class_t *)item1->value;
                if (sy_execute_id_cmp(class2->key, name) == 1)
                {
                    sy_strip_t *new_strip = sy_strip_create(strip);
                    if (new_strip == ERROR)
                    {
                        return ERROR;
                    }
                    return sy_record_make_type(item1, new_strip);
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_FUN)
            {
                sy_node_fun_t *fun1 = (sy_node_fun_t *)item1->value;
                if (sy_execute_id_cmp(fun1->key, name) == 1)
                {
                    sy_strip_t *new_strip = sy_strip_create(strip);
                    if (new_strip == ERROR)
                    {
                        return ERROR;
                    }
                    return sy_record_make_type(item1, new_strip);  
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                sy_node_property_t *property1 = (sy_node_property_t *)item1->value;
                if (sy_execute_id_cmp(property1->key, name) == 1)
                {
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
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)property1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return ERROR;
                        }
                        return entry->value;
                    }
                    else
                    {
                        sy_entry_t *entry = sy_strip_variable_find(strip, base, property1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)property1->key->value;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                if (applicant->id != base->id)
                {
                    continue;
                }

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
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)using1->path->value;
                                sy_module_entry_t *entry = sy_module_load(basic1->value);
                                if (entry == ERROR)
                                {
                                    return ERROR;
                                }
                                
                                sy_node_t *address = NULL;
                                if (package1->value)
                                {
                                    address = package1->value;
                                }
                                else
                                {
                                    address = package1->key;
                                }

                                sy_record_t *result = sy_execute_expression(address, strip, base, entry->root);
                                if (result == ERROR)
                                {
                                    return ERROR;
                                }
                                else
                                if (result != NULL)
                                {
                                    return result;
                                }
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
                    if (applicant->id != base->id)
                    {
                        if ((class1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            sy_error_type_by_node(name, "private access");
                            return ERROR;
                        }
                    }

                    sy_strip_t *new_strip = sy_strip_create(strip);
                    if (new_strip == ERROR)
                    {
                        return ERROR;
                    }

                    return sy_record_make_type(item1, new_strip); 
                }
                
                continue;
            }
            else
            if (item1->kind == NODE_KIND_FOR)
            {
                if (applicant->id != base->id)
                {
                    continue;
                }

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
                        if (applicant->id != base->id)
                        {
                            if ((var1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                            {
                                sy_error_type_by_node(name, "private access");
                                return ERROR;
                            }
                        }

                        sy_entry_t *entry = sy_symbol_table_find(base, var1->key);
                        if (entry == ERROR)
                        {
                            return ERROR;
                        }
                        else
                        if (entry == NULL)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key;
                            sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
                                if (applicant->id != base->id)
                                {
                                    if ((entity1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        sy_error_type_by_node(name, "private access");
                                        return ERROR;
                                    }
                                }

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
                                        sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                                        return ERROR;
                                    }
                                    return entry->value;
                                }
                                else
                                {
                                    sy_entry_t *entry = sy_strip_variable_find(strip, base, entity1->key);
                                    if (entry == ERROR)
                                    {
                                        return ERROR;
                                    }
                                    else
                                    if (entry == NULL)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key;
                                        sy_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
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
        return sy_execute_selection(base->parent, name, strip, applicant);
    }

    sy_node_basic_t *basic1 = (sy_node_basic_t *)name->value;
    sy_error_type_by_node(name, "'%s' not defined", basic1->value);
    return ERROR;
}


sy_record_t *
sy_execute_id(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (origin)
    {
        return sy_execute_selection(origin, node, strip, applicant);
    }
    return sy_execute_selection(node->parent, node, strip, applicant);
}

sy_record_t *
sy_execute_number(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
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
sy_execute_char(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_basic_t *basic1 = (sy_node_basic_t *)node->value;
    char *str = basic1->value;

    return sy_record_make_char((*(char *)str));
}

sy_record_t *
sy_execute_string(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_basic_t *basic1 = (sy_node_basic_t *)node->value;
    return sy_record_make_string(basic1->value);
}

sy_record_t *
sy_execute_null(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_null();
}


sy_record_t *
sy_execute_kint8(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kint16(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kint32(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kint64(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kuint8(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kuint16(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kuint32(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kuint64(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kbigint(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kfloat32(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kfloat64(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kbigfloat(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kchar(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_kstring(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    return sy_record_make_type(node, NULL);
}

sy_record_t *
sy_execute_lambda(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_lambda_t *fun1 = (sy_node_lambda_t *)node->value;
    if (fun1->body)
    {
        sy_strip_t *new_strip = sy_strip_create(strip);
        if (new_strip == ERROR)
        {
            return ERROR;
        }
        return sy_record_make_type(node, new_strip);
    }
    else
    {
        return sy_record_make_type(node, NULL);
    }
}

sy_record_t *
sy_execute_parenthesis(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_unary_t *unary1 = (sy_node_unary_t *)node->value;

    return sy_execute_expression(unary1->right, strip, applicant, origin);
}

sy_record_t *
sy_execute_tuple(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

    int32_t is_tuple = 0;

    sy_record_tuple_t *top = NULL;
    sy_record_tuple_t *declaration = NULL;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        sy_record_t *record_value = sy_execute_expression(item, strip, applicant, NULL);
        if (record_value == ERROR)
        {
            if (top)
            {
                if (sy_record_tuple_destroy(top) < 0)
                {
                    return NULL;
                }
            }
            return NULL;
        }

        if (record_value->kind != RECORD_KIND_TYPE)
        {
            is_tuple = 1;
        }

        record_value->link = 1;
        sy_record_tuple_t *tuple = sy_record_make_tuple(record_value, NULL);
        if (tuple == ERROR)
        {
            record_value->link = 0;
            if (top)
            {
                if (sy_record_tuple_destroy(top) < 0)
                {
                    if (record_value->kind == 0)
                    {
                        if (sy_record_destroy(record_value) < 0)
                        {
                            return NULL;
                        }
                    }
                    return NULL;
                }
            }

            if (record_value->kind == 0)
            {
                if (sy_record_destroy(record_value) < 0)
                {
                    return NULL;
                }
            }
            return NULL;
        }

        if (declaration)
        {
            declaration->next = tuple;
            declaration = tuple;
        }
        else
        {
            declaration = tuple;
            top = tuple;
        }
    }

    if (is_tuple)
    {
        sy_record_t *result = sy_record_create(RECORD_KIND_TUPLE, top);
        if (result == ERROR)
        {
            if (top)
            {
                if (sy_record_tuple_destroy(top) < 0)
                {
                    return NULL;
                }
            }
            return NULL;
        }
        
        return result;
    }
    else
    {
        sy_record_t *result = sy_record_make_type(node, top);
        if (result == ERROR)
        {
            if (top)
            {
                if (sy_record_tuple_destroy(top) < 0)
                {
                    return NULL;
                }
            }
            return NULL;
        }

        return result;
    }
}

sy_record_t *
sy_execute_object(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

    int32_t is_object = 0;
    sy_record_object_t *top = NULL;
    sy_record_object_t *declaration = NULL;
    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        sy_node_pair_t *pair = (sy_node_pair_t *)item->value;

        sy_record_t *record_value = sy_execute_expression(pair->value, strip, applicant, NULL);
        if (record_value == ERROR)
        {
            if (top)
            {
                if (sy_record_object_destroy(top) < 0)
                {
                    return NULL;
                }
            }
            return NULL;
        }

        if (record_value->kind != RECORD_KIND_TYPE)
        {
            is_object = 1;
        }

        record_value->link = 1;
        sy_record_object_t *object = sy_record_make_object(pair->key, record_value, NULL);
        if (object == ERROR)
        {
            record_value->link = 0;
            if (top)
            {
                if (sy_record_object_destroy(top) < 0)
                {
                    if (record_value->kind == 0)
                    {
                        if (sy_record_destroy(record_value) < 0)
                        {
                            return NULL;
                        }
                    }
                    return NULL;
                }
            }

            if (record_value->kind == 0)
            {
                if (sy_record_destroy(record_value) < 0)
                {
                    return NULL;
                }
            }
            return NULL;
        }

        if (declaration)
        {
            declaration->next = object;
            declaration = object;
        }
        else
        {
            declaration = object;
            top = object;
        }
    }

    if (is_object)
    {
        sy_record_t *result = sy_record_create(RECORD_KIND_OBJECT, top);
        if (result == ERROR)
        {
            if (top)
            {
                if (sy_record_object_destroy(top) < 0)
                {
                    return NULL;
                }
            }
            return NULL;
        }

        return result;
    }
    else
    {
        sy_record_t *result = sy_record_make_type(node, top);
        if (result == ERROR)
        {
            if (top)
            {
                if (sy_record_object_destroy(top) < 0)
                {
                    return NULL;
                }
            }
            return NULL;
        }

        return result;
    }
}

sy_record_t *
sy_execute_primary(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_ID)
    {
        return sy_execute_id(node, strip, applicant, origin);
    }
    else

    if (node->kind == NODE_KIND_NUMBER)
    {
        return sy_execute_number(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_CHAR)
    {
        return sy_execute_char(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_STRING)
    {
        return sy_execute_string(node, strip, applicant, origin);
    }
    else

    if (node->kind == NODE_KIND_NULL)
    {
        return sy_execute_null(node, strip, applicant, origin);
    }
    else

    if (node->kind == NODE_KIND_KINT8)
    {
        return sy_execute_kint8(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KINT16)
    {
        return sy_execute_kint16(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KINT32)
    {
        return sy_execute_kint32(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KINT64)
    {
        return sy_execute_kint64(node, strip, applicant, origin);
    }
    else

    if (node->kind == NODE_KIND_KUINT8)
    {
        return sy_execute_kuint8(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KUINT16)
    {
        return sy_execute_kuint16(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KUINT32)
    {
        return sy_execute_kuint32(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KUINT64)
    {
        return sy_execute_kuint64(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KBIGINT)
    {
        return sy_execute_kbigint(node, strip, applicant, origin);
    }
    else

    if (node->kind == NODE_KIND_KFLOAT32)
    {
        return sy_execute_kfloat32(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KFLOAT64)
    {
        return sy_execute_kfloat64(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KBIGFLOAT)
    {
        return sy_execute_kbigfloat(node, strip, applicant, origin);
    }
    else

    if (node->kind == NODE_KIND_KCHAR)
    {
        return sy_execute_kchar(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_KSTRING)
    {
        return sy_execute_kstring(node, strip, applicant, origin);
    }
    else

    if (node->kind == NODE_KIND_TUPLE)
    {
        return sy_execute_tuple(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_OBJECT)
    {
        return sy_execute_object(node, strip, applicant, origin);
    }
    else

    if (node->kind == NODE_KIND_LAMBDA)
    {
        return sy_execute_lambda(node, strip, applicant, origin);
    }
    else
    if (node->kind == NODE_KIND_PARENTHESIS)
    {
        return sy_execute_parenthesis(node, strip, applicant, origin);
    }
    
    sy_error_type_by_node(node, "primary implement not support %d", node->kind);
    return ERROR;
}
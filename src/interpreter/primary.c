#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdint.h>
#include <float.h>
#include <jansson.h>
#include <ffi.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../error.h"
#include "../mutex.h"
#include "../memory.h"
#include "../config.h"
#include "../scanner/scanner.h"
#include "../parser/syntax/syntax.h"
#include "record.h"
#include "../repository.h"
#include "../interpreter.h"
#include "../thread.h"
#include "symbol_table.h"
#include "strip.h"
#include "entry.h"
#include "helper.h"
#include "execute.h"

not_node_t *
not_primary_get_root(not_node_t *base)
{
    if (base && base->kind != NODE_KIND_MODULE)
    {
        return not_primary_get_root(base->parent);
    }

    return base;
}

not_record_t *
not_primary_selection(not_node_t *base, not_node_t *sub, not_node_t *name, not_strip_t *strip, not_node_t *applicant)
{
    if (base->kind == NODE_KIND_CATCH)
    {
        not_node_catch_t *catch1 = (not_node_catch_t *)base->value;

        not_node_t *node1 = catch1->parameters;
        not_node_block_t *block1 = (not_node_block_t *)node1->value;

        for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                not_node_parameter_t *parameter1 = (not_node_parameter_t *)item1->value;
                if (not_helper_id_cmp(parameter1->key, name) == 0)
                {
                    not_entry_t *entry = not_strip_variable_find(strip, base, parameter1->key);
                    if (entry == NOT_PTR_ERROR)
                    {
                        return NOT_PTR_ERROR;
                    }
                    else if (entry == NULL)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)parameter1->key->value;
                        not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                        return NOT_PTR_ERROR;
                    }

                    return entry->value;
                }
            }
        }
    }
    else if (base->kind == NODE_KIND_FOR)
    {
        not_node_for_t *for1 = (not_node_for_t *)base->value;

        for (not_node_t *item1 = for1->initializer; item1 != NULL; item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_VAR)
            {
                not_node_var_t *var1 = (not_node_var_t *)item1->value;
                if (var1->key->kind == NODE_KIND_ID)
                {
                    if (not_helper_id_cmp(var1->key, name) == 0)
                    {
                        assert(strip != NULL);
                        not_entry_t *entry = not_strip_variable_find(strip, base, var1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }
                        return entry->value;
                    }
                }
                else
                {
                    not_node_t *node1 = var1->key;
                    not_node_block_t *block1 = (not_node_block_t *)node1->value;

                    for (not_node_t *item2 = block1->items; item2 != NULL; item2 = item2->next)
                    {
                        if (item2->kind == NODE_KIND_ENTITY)
                        {
                            not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                            if (not_helper_id_cmp(entity1->key, name) == 0)
                            {
                                assert(strip != NULL);
                                not_entry_t *entry = not_strip_variable_find(strip, base, entity1->key);
                                if (entry == NOT_PTR_ERROR)
                                {
                                    return NOT_PTR_ERROR;
                                }
                                else if (entry == NULL)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;
                                    not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                                    return NOT_PTR_ERROR;
                                }

                                return entry->value;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (base->kind == NODE_KIND_FORIN)
    {
        not_node_forin_t *for1 = (not_node_forin_t *)base->value;

        if (for1->field)
        {
            if (not_helper_id_cmp(for1->field, name) == 0)
            {
                not_entry_t *entry = not_strip_variable_find(strip, base, for1->field);
                if (entry == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->field->value;
                    not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                    return NOT_PTR_ERROR;
                }
                return entry->value;
            }
        }

        if (for1->value)
        {
            if (not_helper_id_cmp(for1->value, name) == 0)
            {
                not_entry_t *entry = not_strip_variable_find(strip, base, for1->value);
                if (entry == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->value->value;
                    not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                    return NOT_PTR_ERROR;
                }
                return entry->value;
            }
        }
    }
    else if (base->kind == NODE_KIND_BODY)
    {
        not_node_block_t *block1 = (not_node_block_t *)base->value;

        for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
        {
            if (sub && (item1->id == sub->id))
            {
                break;
            }

            if (item1->kind == NODE_KIND_FOR)
            {
                not_node_for_t *for1 = (not_node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (not_helper_id_cmp(for1->key, name) == 0)
                    {
                        return not_record_make_type(item1, NULL);
                    }
                }
            }
            else if (item1->kind == NODE_KIND_FORIN)
            {
                not_node_forin_t *for1 = (not_node_forin_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (not_helper_id_cmp(for1->key, name) == 0)
                    {
                        return not_record_make_type(item1, NULL);
                    }
                }
            }
            else if (item1->kind == NODE_KIND_VAR)
            {
                not_node_var_t *var1 = (not_node_var_t *)item1->value;
                if (var1->key->kind == NODE_KIND_ID)
                {
                    if (not_helper_id_cmp(var1->key, name) == 0)
                    {
                        assert(strip != NULL);
                        not_entry_t *entry = not_strip_variable_find(strip, base, var1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                }
                else
                {
                    not_node_t *node2 = var1->key;
                    not_node_block_t *block2 = (not_node_block_t *)node2->value;

                    for (not_node_t *item2 = block2->items; item2 != NULL; item2 = item2->next)
                    {
                        if (item2->kind == NODE_KIND_ENTITY)
                        {
                            not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                            if (not_helper_id_cmp(entity1->key, name) == 0)
                            {
                                assert(strip != NULL);
                                not_entry_t *entry = not_strip_variable_find(strip, base, entity1->key);
                                if (entry == NOT_PTR_ERROR)
                                {
                                    return NOT_PTR_ERROR;
                                }
                                else if (entry == NULL)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;
                                    not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                                    return NOT_PTR_ERROR;
                                }

                                return entry->value;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (base->kind == NODE_KIND_LAMBDA)
    {
        not_node_lambda_t *fun1 = (not_node_lambda_t *)base->value;

        if (fun1->key)
        {
            if (not_helper_id_cmp(fun1->key, name) == 0)
            {
                not_strip_t *strip_copy = not_strip_create(NULL);
                if (strip_copy == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }

                return not_record_make_type(base, strip_copy);
            }
        }

        if (fun1->generics != NULL)
        {
            not_node_t *node1 = fun1->generics;
            not_node_block_t *block1 = (not_node_block_t *)node1->value;

            for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;
                    if (not_helper_id_cmp(generic1->key, name) == 0)
                    {
                        assert(strip != NULL);
                        not_entry_t *entry = not_strip_variable_find(strip, base, generic1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                }
            }
        }

        if (fun1->parameters != NULL)
        {
            not_node_t *node1 = fun1->parameters;
            not_node_block_t *block1 = (not_node_block_t *)node1->value;

            for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    not_node_parameter_t *parameter1 = (not_node_parameter_t *)item1->value;
                    if (not_helper_id_cmp(parameter1->key, name) == 0)
                    {
                        not_entry_t *entry = not_strip_variable_find(strip, base, parameter1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)parameter1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                }
            }
        }
    }
    else if (base->kind == NODE_KIND_FUN)
    {
        not_node_fun_t *fun1 = (not_node_fun_t *)base->value;

        if (fun1->generics != NULL)
        {
            not_node_t *node1 = fun1->generics;
            not_node_block_t *block1 = (not_node_block_t *)node1->value;

            for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;
                    if (not_helper_id_cmp(generic1->key, name) == 0)
                    {
                        assert(strip != NULL);
                        not_entry_t *entry = not_strip_variable_find(strip, base, generic1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                }
            }
        }

        if (fun1->parameters != NULL)
        {
            not_node_t *node1 = fun1->parameters;
            not_node_block_t *block1 = (not_node_block_t *)node1->value;

            for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    not_node_parameter_t *parameter1 = (not_node_parameter_t *)item1->value;
                    if (not_helper_id_cmp(parameter1->key, name) == 0)
                    {
                        not_entry_t *entry = not_strip_variable_find(strip, base, parameter1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)parameter1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                }
            }
        }
    }
    else if (base->kind == NODE_KIND_CLASS)
    {
        not_node_class_t *class1 = (not_node_class_t *)base->value;

        if (class1->generics != NULL)
        {
            not_node_t *node1 = class1->generics;
            not_node_block_t *block1 = (not_node_block_t *)node1->value;

            for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;
                    if (not_helper_id_cmp(generic1->key, name) == 0)
                    {
                        not_entry_t *entry = not_strip_variable_find(strip, base, generic1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                }
            }
        }

        if (class1->heritages != NULL)
        {
            not_node_t *node1 = class1->heritages;
            not_node_block_t *block1 = (not_node_block_t *)node1->value;

            for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
            {
                if (item1->kind == NODE_KIND_HERITAGE)
                {
                    not_node_heritage_t *heritage1 = (not_node_heritage_t *)item1->value;
                    if (not_helper_id_cmp(heritage1->key, name) == 0)
                    {
                        assert(strip != NULL);
                        not_entry_t *entry = not_strip_variable_find(strip, base, heritage1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)heritage1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                }
            }
        }

        for (not_node_t *item1 = class1->block; item1 != NULL; item1 = item1->next)
        {
            if (sub && (item1->id == sub->id) && (sub->kind == NODE_KIND_PROPERTY))
            {
                break;
            }

            if (item1->kind == NODE_KIND_CLASS)
            {
                not_node_class_t *class2 = (not_node_class_t *)item1->value;
                if (not_helper_id_cmp(class2->key, name) == 0)
                {
                    not_strip_t *strip_copy = not_strip_create(NULL);
                    if (strip_copy == NOT_PTR_ERROR)
                    {
                        return NOT_PTR_ERROR;
                    }

                    return not_record_make_type(item1, strip_copy);
                }
                continue;
            }
            else if (item1->kind == NODE_KIND_FUN)
            {
                not_node_fun_t *fun1 = (not_node_fun_t *)item1->value;
                if (not_helper_id_cmp(fun1->key, name) == 0)
                {
                    not_strip_t *strip_copy = not_strip_create(NULL);
                    if (strip_copy == NOT_PTR_ERROR)
                    {
                        return NOT_PTR_ERROR;
                    }

                    return not_record_make_type(item1, strip_copy);
                }
                continue;
            }
            else if (item1->kind == NODE_KIND_PROPERTY)
            {
                not_node_property_t *property1 = (not_node_property_t *)item1->value;
                if (not_helper_id_cmp(property1->key, name) == 0)
                {
                    if ((property1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                    {
                        not_entry_t *entry = not_symbol_table_find(base, property1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)property1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                    else
                    {
                        not_entry_t *entry = not_strip_variable_find(strip, base, property1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)property1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                }
                continue;
            }
        }
    }
    else if (base->kind == NODE_KIND_MODULE)
    {
        not_node_block_t *block1 = (not_node_block_t *)base->value;

        for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_USING)
            {
                if (not_primary_get_root(name)->id != base->id)
                {
                    continue;
                }

                not_node_using_t *using1 = (not_node_using_t *)item1->value;

                if (using1->packages != NULL)
                {
                    not_node_t *node1 = using1->packages;
                    not_node_block_t *block2 = (not_node_block_t *)node1->value;

                    for (not_node_t *item2 = block2->items; item2 != NULL; item2 = item2->next)
                    {
                        if (item2->kind == NODE_KIND_PACKAGE)
                        {
                            not_node_package_t *package1 = (not_node_package_t *)item2->value;
                            if (not_helper_id_cmp(package1->key, name) == 0)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)using1->path->value;

                                not_module_t *entry = not_repository_load(using1->base, basic1->value);
                                if (entry == NOT_PTR_ERROR)
                                {
                                    return NOT_PTR_ERROR;
                                }

                                not_node_t *address = NULL;
                                if (package1->value)
                                {
                                    address = package1->value;
                                }
                                else
                                {
                                    address = package1->key;
                                }

                                if (entry->handle)
                                {
                                    if (address->kind != NODE_KIND_ID)
                                    {
                                        not_node_basic_t *basic1 = (not_node_basic_t *)name->value;
                                        not_error_type_by_node(address, "'%s' not a static proc address", basic1->value);
                                        return NOT_PTR_ERROR;
                                    }

                                    not_node_basic_t *basic = (not_node_basic_t *)address->value;

                                    json_t *json_root = NULL;
                                    json_t *json_funs = json_object_get(entry->json, "functions");
                                    if (json_is_array(json_funs))
                                    {
                                        json_t *json_fun;
                                        size_t index;
                                        json_array_foreach(json_funs, index, json_fun)
                                        {
                                            json_t *json_fun_name = json_object_get(json_fun, "name");
                                            if (json_is_string(json_fun_name))
                                            {
                                                if (strcmp(json_string_value(json_fun_name), basic->value) == 0)
                                                {
                                                    json_root = json_fun;
                                                    break;
                                                }
                                            }
                                        }
                                    }

                                    if (!json_root)
                                    {
                                        not_node_basic_t *basic1 = (not_node_basic_t *)name->value;
                                        not_error_type_by_node(address, "'json' it does not have '%s'", basic1->value);
                                        return NOT_PTR_ERROR;
                                    }

#ifdef _WIN32
                                    void *proc = GetProcAddress(entry->handle, basic->value);
                                    if (!proc)
                                    {
                                        not_error_type_by_node(name, "using '%s' failed to find function", basic->value);
                                        return NOT_PTR_ERROR;
                                    }
#else
                                    void *proc = dlsym(entry->handle, basic->value);
                                    char *error;
                                    if ((error = dlerror()) != NULL)
                                    {
                                        not_error_type_by_node(name, "%s", error);
                                        return NOT_PTR_ERROR;
                                    }
#endif

                                    return not_record_make_proc(proc, json_root);
                                }

                                not_record_t *result = not_expression(address, strip, base, entry->root);
                                if (result == NOT_PTR_ERROR)
                                {
                                    return NOT_PTR_ERROR;
                                }
                                else if (result != NULL)
                                {
                                    return result;
                                }
                            }
                        }
                    }
                }

                continue;
            }
            else if (item1->kind == NODE_KIND_CLASS)
            {
                not_node_class_t *class1 = (not_node_class_t *)item1->value;

                if (not_helper_id_cmp(class1->key, name) == 0)
                {
                    if (applicant->id != base->id)
                    {
                        if ((class1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            not_error_type_by_node(name, "private access");
                            return NOT_PTR_ERROR;
                        }
                    }

                    not_strip_t *strip_copy = not_strip_create(NULL);
                    if (strip_copy == NOT_PTR_ERROR)
                    {
                        return NOT_PTR_ERROR;
                    }

                    return not_record_make_type(item1, strip_copy);
                }

                continue;
            }
            else if (item1->kind == NODE_KIND_FOR)
            {
                if (applicant->id != base->id)
                {
                    continue;
                }

                not_node_for_t *for1 = (not_node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (not_helper_id_cmp(for1->key, name) == 0)
                    {
                        return not_record_make_type(item1, NULL);
                    }
                }
            }
            else if (item1->kind == NODE_KIND_FORIN)
            {
                if (applicant->id != base->id)
                {
                    continue;
                }

                not_node_forin_t *for1 = (not_node_forin_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (not_helper_id_cmp(for1->key, name) == 0)
                    {
                        return not_record_make_type(item1, NULL);
                    }
                }
            }
            else if (item1->kind == NODE_KIND_VAR)
            {
                not_node_var_t *var1 = (not_node_var_t *)item1->value;
                if (var1->key->kind == NODE_KIND_ID)
                {
                    if (not_helper_id_cmp(var1->key, name) == 0)
                    {
                        if (applicant->id != base->id)
                        {
                            if ((var1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                            {
                                not_error_type_by_node(name, "private access");
                                return NOT_PTR_ERROR;
                            }
                        }

                        not_entry_t *entry = not_symbol_table_find(base, var1->key);
                        if (entry == NOT_PTR_ERROR)
                        {
                            return NOT_PTR_ERROR;
                        }
                        else if (entry == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;
                            not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                            return NOT_PTR_ERROR;
                        }

                        return entry->value;
                    }
                }
                else
                {
                    not_node_t *node2 = var1->key;
                    not_node_block_t *block2 = (not_node_block_t *)node2->value;

                    for (not_node_t *item2 = block2->items; item2 != NULL; item2 = item2->next)
                    {
                        if (item2->kind == NODE_KIND_ENTITY)
                        {
                            not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                            if (not_helper_id_cmp(entity1->key, name) == 0)
                            {
                                if (applicant->id != base->id)
                                {
                                    if ((entity1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        not_error_type_by_node(name, "private access");
                                        return NOT_PTR_ERROR;
                                    }
                                }

                                not_entry_t *entry = not_symbol_table_find(base, entity1->key);
                                if (entry == NOT_PTR_ERROR)
                                {
                                    return NOT_PTR_ERROR;
                                }
                                else if (entry == NULL)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;
                                    not_error_runtime_by_node(name, "'%s' is not initialized", basic1->value);
                                    return NOT_PTR_ERROR;
                                }

                                return entry->value;
                            }
                        }
                    }
                }
            }
        }
    }

    if (base->parent != NULL)
    {
        return not_primary_selection(base->parent, base, name, strip, applicant);
    }

    not_node_basic_t *basic1 = (not_node_basic_t *)name->value;
    not_error_type_by_node(name, "'%s' not defined", basic1->value);
    return NOT_PTR_ERROR;
}

not_record_t *
not_primary_id(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (origin)
    {
        return not_primary_selection(origin, node, node, strip, applicant);
    }
    return not_primary_selection(node->parent, node, node, strip, applicant);
}

not_record_t *
not_primary_number(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_basic_t *basic1 = (not_node_basic_t *)node->value;
    char *str = basic1->value;

    int32_t base = 10;

    if (str[0] == '0')
    {
        if (str[1] == 'x' || str[1] == 'X')
        {
            base = 16;
            str += 2;
        }
        else if (str[1] == 'b' || str[1] == 'B')
        {
            base = 2;
            str += 2;
        }
        else if (isdigit(str[1]))
        {
            if (not_utils_is_octal_string(str))
            {
                base = 8;
            }
            else
            {
                base = 10;
            }
        }
    }

    not_record_t *record = NULL;

    if (strchr(str, '.'))
    {
        mpf_t result;
        mpf_init(result);
        mpf_set_str(result, str, base);
        record = not_record_make_float_from_f(result);
        mpf_clear(result);
    }
    else
    {
        mpz_t result_mpz;
        mpz_init(result_mpz);
        mpz_set_str(result_mpz, str, base);
        record = not_record_make_int_from_z(result_mpz);
        mpz_clear(result_mpz);
    }

    return record;
}

not_record_t *
not_primary_char(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_basic_t *basic1 = (not_node_basic_t *)node->value;
    char *str = basic1->value;

    return not_record_make_char((*(char *)str));
}

not_record_t *
not_primary_string(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_basic_t *basic = (not_node_basic_t *)node->value;
    return not_record_make_string(basic->value);
}

not_record_t *
not_primary_null(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    return not_record_make_null();
}

not_record_t *
not_primary_undefined(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    return not_record_make_undefined();
}

not_record_t *
not_primary_nan(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    return not_record_make_nan();
}

not_record_t *
not_primary_kint(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    return not_record_make_type(node, NULL);
}

not_record_t *
not_primary_kfloat(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    return not_record_make_type(node, NULL);
}

not_record_t *
not_primary_kchar(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    return not_record_make_type(node, NULL);
}

not_record_t *
not_primary_kstring(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    return not_record_make_type(node, NULL);
}

not_record_t *
not_primary_this(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_t *iter = node->parent;
    while (iter)
    {
        if (iter->kind == NODE_KIND_CLASS)
        {
            break;
        }
        iter = iter->parent;
    }

    if (iter)
    {
        not_strip_t *strip_copy = not_strip_copy(strip);
        if (strip_copy == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        return not_record_make_struct(iter, strip_copy);
    }
    else
    {
        return not_record_make_undefined();
    }
}

not_record_t *
not_primary_lambda(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_lambda_t *fun1 = (not_node_lambda_t *)node->value;
    if (fun1->body)
    {
        not_strip_t *strip_copy = not_strip_create(NULL);
        if (strip_copy == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        return not_record_make_type(node, strip_copy);
    }
    else
    {
        return not_record_make_type(node, NULL);
    }
}

not_record_t *
not_primary_parenthesis(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_unary_t *unary1 = (not_node_unary_t *)node->value;

    return not_expression(unary1->right, strip, applicant, origin);
}

not_record_t *
not_primary_tuple(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    int32_t is_tuple = 0;

    not_record_tuple_t *top = NULL;
    not_record_tuple_t *declaration = NULL;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        not_record_t *record_value = not_expression(item, strip, applicant, NULL);
        if (record_value == NOT_PTR_ERROR)
        {
            if (top)
            {
                if (not_record_tuple_destroy(top) < 0)
                {
                    return NOT_PTR_ERROR;
                }
            }
            return NOT_PTR_ERROR;
        }

        if (record_value->kind != RECORD_KIND_TYPE)
        {
            is_tuple = 1;
        }

        not_record_tuple_t *tuple = not_record_make_tuple(record_value, NULL);
        if (tuple == NOT_PTR_ERROR)
        {
            if (top)
            {
                if (not_record_tuple_destroy(top) < 0)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return NOT_PTR_ERROR;
                    }
                    return NOT_PTR_ERROR;
                }
            }

            if (not_record_link_decrease(record_value) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return NOT_PTR_ERROR;
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

    if (is_tuple || (top == NULL))
    {
        not_record_t *result = not_record_create(RECORD_KIND_TUPLE, top);
        if (result == NOT_PTR_ERROR)
        {
            if (top)
            {
                if (not_record_tuple_destroy(top) < 0)
                {
                    return NOT_PTR_ERROR;
                }
            }
            return NOT_PTR_ERROR;
        }

        return result;
    }
    else
    {
        not_record_t *result = not_record_make_type(node, top);
        if (result == NOT_PTR_ERROR)
        {
            if (top)
            {
                if (not_record_tuple_destroy(top) < 0)
                {
                    return NOT_PTR_ERROR;
                }
            }
            return NOT_PTR_ERROR;
        }

        return result;
    }
}

not_record_t *
not_primary_object(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    int32_t is_object = 0;
    not_record_object_t *top = NULL;
    not_record_object_t *declaration = NULL;
    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        not_node_pair_t *pair = (not_node_pair_t *)item->value;

        not_record_t *record_value = NULL;

        if (pair->value)
        {
            record_value = not_expression(pair->value, strip, applicant, NULL);
            if (record_value == NOT_PTR_ERROR)
            {
                if (top)
                {
                    if (not_record_object_destroy(top) < 0)
                    {
                        return NOT_PTR_ERROR;
                    }
                }
                return NOT_PTR_ERROR;
            }
        }
        else
        {
            record_value = not_expression(pair->key, strip, applicant, NULL);
            if (record_value == NOT_PTR_ERROR)
            {
                if (top)
                {
                    if (not_record_object_destroy(top) < 0)
                    {
                        return NOT_PTR_ERROR;
                    }
                }
                return NOT_PTR_ERROR;
            }
        }

        if (record_value->kind != RECORD_KIND_TYPE)
        {
            is_object = 1;
        }

        not_node_basic_t *basic = (not_node_basic_t *)pair->key->value;
        not_record_object_t *object = not_record_make_object(basic->value, record_value, NULL);
        if (object == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(record_value) < 0)
            {
                return NOT_PTR_ERROR;
            }
            if (top)
            {
                if (not_record_object_destroy(top) < 0)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return NOT_PTR_ERROR;
                    }
                    return NOT_PTR_ERROR;
                }
            }

            if (not_record_link_decrease(record_value) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return NOT_PTR_ERROR;
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

    if (is_object || (top == NULL))
    {
        not_record_t *result = not_record_create(RECORD_KIND_OBJECT, top);
        if (result == NOT_PTR_ERROR)
        {
            if (top)
            {
                if (not_record_object_destroy(top) < 0)
                {
                    return NOT_PTR_ERROR;
                }
            }
            return NOT_PTR_ERROR;
        }

        return result;
    }
    else
    {
        not_record_t *result = not_record_make_type(node, top);
        if (result == NOT_PTR_ERROR)
        {
            if (top)
            {
                if (not_record_object_destroy(top) < 0)
                {
                    return NOT_PTR_ERROR;
                }
            }
            return NOT_PTR_ERROR;
        }

        return result;
    }
}

not_record_t *
not_primary(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_ID)
    {
        return not_primary_id(node, strip, applicant, origin);
    }
    else

        if (node->kind == NODE_KIND_NUMBER)
    {
        return not_primary_number(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_CHAR)
    {
        return not_primary_char(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_STRING)
    {
        return not_primary_string(node, strip, applicant, origin);
    }
    else

        if (node->kind == NODE_KIND_NULL)
    {
        return not_primary_null(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_UNDEFINED)
    {
        return not_primary_undefined(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_NAN)
    {
        return not_primary_nan(node, strip, applicant, origin);
    }
    else

        if (node->kind == NODE_KIND_THIS)
    {
        return not_primary_this(node, strip, applicant, origin);
    }
    else

        if (node->kind == NODE_KIND_KINT)
    {
        return not_primary_kint(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_KFLOAT)
    {
        return not_primary_kfloat(node, strip, applicant, origin);
    }
    else

        if (node->kind == NODE_KIND_KCHAR)
    {
        return not_primary_kchar(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_KSTRING)
    {
        return not_primary_kstring(node, strip, applicant, origin);
    }
    else

        if (node->kind == NODE_KIND_TUPLE)
    {
        return not_primary_tuple(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_OBJECT)
    {
        return not_primary_object(node, strip, applicant, origin);
    }
    else

        if (node->kind == NODE_KIND_LAMBDA)
    {
        return not_primary_lambda(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_PARENTHESIS)
    {
        return not_primary_parenthesis(node, strip, applicant, origin);
    }

    not_error_type_by_node(node, "primary implement not support %d", node->kind);
    return NOT_PTR_ERROR;
}
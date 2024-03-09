#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/file.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "parser.h"
#include "error.h"
#include "syntax.h"

static error_t *
syntax_error(program_t *program, node_t *node, const char *format, ...)
{
	char *message;
	message = malloc(1024);
	if (!message)
	{
		return NULL;
	}

	va_list arg;
	if (format)
	{
		va_start(arg, format);
		vsprintf(message, format, arg);
		va_end(arg);
	}

	error_t *error;
	error = error_create(node->position, message);
	if (!error)
	{
		return NULL;
	}

	if (list_rpush(program->errors, error))
	{
		return NULL;
	}

	return error;
}

static int32_t
syntax_idcmp(node_t *n1, node_t *n2)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	node_basic_t *nb2 = (node_basic_t *)n2->value;

	//printf("%s %s\n", nb1->value, nb2->value);

	return strcmp(nb1->value, nb2->value);
}

static int32_t
syntax_idstrcmp(node_t *n1, char *name)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	return strcmp(nb1->value, name);
}


static int32_t
syntax_expression(program_t *program, node_t *node, list_t *response)
{
    return 1;
}

static int32_t
syntax_func(program_t *program, node_t *node)
{
	//node_import_t *node_import = (node_import_t *)node->value;
	return 1;
}

static int32_t
syntax_enum(program_t *program, node_t *node)
{
	//node_import_t *node_import = (node_import_t *)node->value;
	return 1;
}

static int32_t
syntax_import(program_t *program, node_t *node)
{
	//node_import_t *node_import = (node_import_t *)node->value;
	return 1;
}

static int32_t
syntax_eqaul_gsgs(program_t *program, node_t *ngs1, node_t *ngs2)
{
    return 1;
}

static int32_t
syntax_eqaul_psps(program_t *program, node_t *nps1, node_t *nps2)
{
    return 1;
}

static int32_t
syntax_property(program_t *program, node_t *node)
{
	//node_import_t *node_import = (node_import_t *)node->value;
	return 1;
}

static int32_t
syntax_heritages(program_t *program, node_t *node)
{
	node_block_t *heritages = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = heritages->list->begin;a1 != heritages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_HERITAGE)
        {
            node_generic_t *heritage1 = (node_generic_t *)item1->value;
            ilist_t *a2;
            for (a2 = heritages->list->begin;a2 != heritages->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_HERITAGE)
                {
                    node_generic_t *heritage2 = (node_generic_t *)item2->value;
                    if (item1->id == item2->id)
                    {
                        break;
                    }
                    if (syntax_idcmp(heritage1->key, heritage2->key) == 0)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
            }

            if (heritage1->type != NULL)
            {
                list_t *response1 = list_create();
                if (response1 != NULL)
                {
                    fprintf(stderr, "unable to allocate memory\n");
                    return -1;
                }
                int32_t r1 = syntax_expression(program, heritage1->type, response1);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 0)
                {
                    syntax_error(program, item1, "reference not found");
                    return -1;
                }
                else
                {
                    if (list_count(response1) > 1)
                    {
                        syntax_error(program, item1, "multiple reference");
                        return -1;
                    }
                    list_destroy(response1);
                }
            }
        }
    }
	return 1;
}

static int32_t
syntax_generics(program_t *program, node_t *node)
{
	node_block_t *generics = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = generics->list->begin;a1 != generics->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_GENERIC)
        {
            node_generic_t *generic1 = (node_generic_t *)item1->value;
            ilist_t *a2;
            for (a2 = generics->list->begin;a2 != generics->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic2 = (node_generic_t *)item2->value;
                    if (item1->id == item2->id)
                    {
                        break;
                    }
                    if (syntax_idcmp(generic1->key, generic2->key) == 0)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
            }

            if (generic1->type != NULL)
            {
                list_t *response1 = list_create();
                if (response1 != NULL)
                {
                    fprintf(stderr, "unable to allocate memory\n");
                    return -1;
                }
                int32_t r1 = syntax_expression(program, generic1->type, response1);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 0)
                {
                    syntax_error(program, item1, "reference not found");
                    return -1;
                }
                else
                {
                    if (list_count(response1) > 1)
                    {
                        syntax_error(program, item1, "multiple reference");
                        return -1;
                    }
                    list_destroy(response1);
                }
            }

            if (generic1->value != NULL)
            {
                list_t *response1 = list_create();
                if (response1 != NULL)
                {
                    fprintf(stderr, "unable to allocate memory\n");
                    return -1;
                }
                int32_t r1 = syntax_expression(program, generic1->value, response1);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 0)
                {
                    syntax_error(program, item1, "reference not found");
                    return -1;
                }
                else
                {
                    if (list_count(response1) > 1)
                    {
                        syntax_error(program, item1, "multiple reference");
                        return -1;
                    }
                    list_destroy(response1);
                }
            }
            
        }
    }
	return 1;
}

static int32_t
syntax_objectkey(program_t *program, node_t *node, node_t *key)
{
    if (key->kind == NODE_KIND_OBJECT)
    {
        node_object_t *node_object = (node_object_t *)key->value;
        ilist_t *a1;
        for (a1 = node_object->list->begin;a1 != node_object->list->end;a1 = a1->next)
        {
            node_t *item = (node_t *)a1->value;
            node_property_t *node_property = (node_property_t *)item->value;
            if (node_property->type != NULL)
            {
                int32_t r1 = syntax_objectkey(program, node, node_property->type);
                if (r1 == -1)
                {
                    return -1;
                }
            }
            else
            {
                int32_t r1 = syntax_objectkey(program, node, node_property->key);
                if (r1 == -1)
                {
                    return -1;
                }
            }
        }
    }
    else 
    if (key->kind == NODE_KIND_ID)
    {
        node_class_t *node_class = (node_class_t *)node->value;
        if (syntax_idcmp(node_class->key, key) == 0)
        {
            node_t *ngs1 = node_class->generics;
            node_t *ngs2 = NULL;
            int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
            if (r1 == -1)
            {
                return -1;
            }
            else
            if (r1 == 1)
            {
                node_t *node3 = node_class->block;
                node_block_t *block2 = (node_block_t *)node3->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_FUNC)
                    {
                        node_func_t *fun1 = (node_func_t *)item2->value;
                        if (syntax_idstrcmp(fun1->key, "constructor") == 0)
                        {
                            node_t *nps1 = fun1->parameters;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    key->position.line, key->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 1;
}

static int32_t
syntax_class(program_t *program, node_t *node)
{
	node_class_t *node_class = (node_class_t *)node->value;
    node_t *parent = node->parent;
    region_start:
    if (parent != NULL)
    {
        if (parent->kind == NODE_KIND_MODULE)
        {
            node_module_t *module = (node_module_t *)parent->value;
            node_t *node2 = module->block;
            node_block_t *block = (node_block_t *)node2->value;
            ilist_t *a1;
            for (a1 = block->list->begin;a1 != block->list->end;a1 = a1->next)
            {
                node_t *item = (node_t *)a1->value;
                if (node->id == item->id)
                {
                    break;
                }

                if (item->kind == NODE_KIND_CLASS)
                {
                    node_class_t *node_class2 = (node_class_t *)item->value;
                    if (syntax_idcmp(node_class->key, node_class2->key) == 0)
                    {
                        node_t *ngs1 = node_class->generics;
                        node_t *ngs2 = node_class2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = node_class->block;
                            node_block_t *block2 = (node_block_t *)node3->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item2->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 0)
                                    {
                                        node_t *node4 = node_class2->block;
                                        node_block_t *block3 = (node_block_t *)node4->value;
                                        ilist_t *a3;
                                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a2->next)
                                        {
                                            node_t *item3 = (node_t *)a2->value;
                                            if (item3->kind == NODE_KIND_FUNC)
                                            {
                                                node_func_t *fun2 = (node_func_t *)item3->value;
                                                if (syntax_idstrcmp(fun2->key, "constructor") == 0)
                                                {
                                                    node_t *nps1 = fun1->parameters;
                                                    node_t *nps2 = fun2->parameters;
                                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                                    if (r2 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r2 == 1)
                                                    {
                                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                            item->position.line, item->position.column);
                                                        return -1;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                    }
                }
                else
                if (item->kind == NODE_KIND_FUNC)
                {
                    node_func_t *node_func1 = (node_func_t *)item->value;
                    if (syntax_idcmp(node_class->key, node_func1->key) == 0)
                    {
                        node_t *ngs1 = node_class->generics;
                        node_t *ngs2 = node_func1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = node_class->block;
                            node_block_t *block2 = (node_block_t *)node3->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item2->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 0)
                                    {
                                        node_t *nps1 = fun1->parameters;
                                        node_t *nps2 = node_func1->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item->position.line, item->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *node_enum1 = (node_enum_t *)item->value;
                    if (syntax_idcmp(node_class->key, node_enum1->key) == 0)
                    {
                        node_t *ngs1 = node_class->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = node_class->block;
                            node_block_t *block2 = (node_block_t *)node3->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item2->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 0)
                                    {
                                        node_t *nps1 = fun1->parameters;
                                        node_t *nps2 = NULL;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item->position.line, item->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item->kind == NODE_KIND_VAR)
                {
                    node_var_t *node_var = (node_var_t *)item->value;
                    if (node_var->key->kind == NODE_KIND_ID)
                    {
                        if (syntax_idcmp(node_class->key, node_var->key) == 0)
                        {
                            node_t *ngs1 = node_class->generics;
                            node_t *ngs2 = NULL;
                            int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                node_t *node3 = node_class->block;
                                node_block_t *block2 = (node_block_t *)node3->value;
                                ilist_t *a2;
                                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                                {
                                    node_t *item2 = (node_t *)a2->value;
                                    if (item2->kind == NODE_KIND_FUNC)
                                    {
                                        node_func_t *fun1 = (node_func_t *)item2->value;
                                        if (syntax_idstrcmp(fun1->key, "constructor") == 0)
                                        {
                                            node_t *nps1 = fun1->parameters;
                                            node_t *nps2 = NULL;
                                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                            if (r2 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r2 == 1)
                                            {
                                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                    item->position.line, item->position.column);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        int32_t r1 = syntax_objectkey(program, node, node_var->key);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                    }
                }
            }
        }
        else
        if (parent->kind == NODE_KIND_CLASS)
        {
            node_class_t *class = (node_class_t *)parent->value;
            node_t *node2 = class->block;
            node_block_t *block = (node_block_t *)node2->value;
            ilist_t *a1;
            for (a1 = block->list->begin;a1 != block->list->end;a1 = a1->next)
            {
                node_t *item = (node_t *)a1->value;
                if (node->id == item->id)
                {
                    break;
                }

                if (item->kind == NODE_KIND_CLASS)
                {
                    node_class_t *node_class2 = (node_class_t *)item->value;
                    if (syntax_idcmp(node_class->key, node_class2->key) == 0)
                    {
                        node_t *ngs1 = node_class->generics;
                        node_t *ngs2 = node_class2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = node_class->block;
                            node_block_t *block2 = (node_block_t *)node3->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item2->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 0)
                                    {
                                        node_t *node4 = node_class2->block;
                                        node_block_t *block3 = (node_block_t *)node4->value;
                                        ilist_t *a3;
                                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a2->next)
                                        {
                                            node_t *item3 = (node_t *)a2->value;
                                            if (item3->kind == NODE_KIND_FUNC)
                                            {
                                                node_func_t *fun2 = (node_func_t *)item3->value;
                                                if (syntax_idstrcmp(fun2->key, "constructor") == 0)
                                                {
                                                    node_t *nps1 = fun1->parameters;
                                                    node_t *nps2 = fun2->parameters;
                                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                                    if (r2 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r2 == 1)
                                                    {
                                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                            item->position.line, item->position.column);
                                                        return -1;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                    }
                }
                else
                if (item->kind == NODE_KIND_FUNC)
                {
                    node_func_t *node_func1 = (node_func_t *)item->value;
                    if (syntax_idcmp(node_class->key, node_func1->key) == 0)
                    {
                        node_t *ngs1 = node_class->generics;
                        node_t *ngs2 = node_func1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = node_class->block;
                            node_block_t *block2 = (node_block_t *)node3->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item2->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 0)
                                    {
                                        node_t *nps1 = fun1->parameters;
                                        node_t *nps2 = node_func1->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item->position.line, item->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *node_enum1 = (node_enum_t *)item->value;
                    if (syntax_idcmp(node_class->key, node_enum1->key) == 0)
                    {
                        node_t *ngs1 = node_class->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = node_class->block;
                            node_block_t *block2 = (node_block_t *)node3->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item2->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 0)
                                    {
                                        node_t *nps1 = fun1->parameters;
                                        node_t *nps2 = NULL;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item->position.line, item->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *node_property = (node_property_t *)item->value;
                    if (syntax_idcmp(node_class->key, node_property->key) == 0)
                    {
                        node_t *ngs1 = node_class->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = node_class->block;
                            node_block_t *block2 = (node_block_t *)node3->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item2->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 0)
                                    {
                                        node_t *nps1 = fun1->parameters;
                                        node_t *nps2 = NULL;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item->position.line, item->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (class->generics != NULL)
            {
                node_block_t *generics = (node_generic_t *)class->generics;
                
            }
        }
    }

    if (parent->parent)
    {
        parent = parent->parent;
        goto region_start;
    }

    if (node_class->generics != NULL)
    {
        int32_t r1 = syntax_generics(program, node_class->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (node_class->heritages != NULL)
    {
        int32_t r1 = syntax_heritages(program, node_class->heritages);
        if (r1 == -1)
        {
            return -1;
        }
    }


    node_t *block = node_class->block;
    node_block_t *node_block = (node_block_t *)block->value;
    int32_t constructor_defined = 0;
    ilist_t *a1;
    for (a1 = node_block->list->begin;a1 != node_block->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        if (item->kind == NODE_KIND_IMPORT)
        {
            int32_t result;
            result = syntax_import(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_CLASS)
        {
            int32_t result;
            result = syntax_class(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_ENUM)
        {
            int32_t result;
            result = syntax_enum(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_FUNC)
        {
            int32_t result;
            result = syntax_func(program, item);
            if (result == -1)
            {
                return -1;
            }

            node_func_t *node_func = (node_func_t *)item->value;
            if (syntax_idstrcmp(node_func->key, "constructor") == 0)
            {
                constructor_defined = 1;
            }
        }
        else
        if (item->kind == NODE_KIND_PROPERTY)
        {
            int32_t result;
            result = syntax_property(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
    }

    if (constructor_defined == 0)
    {
        syntax_error(program, node, "constructor not defined");
        return -1;
    }

	return 1;
}

static int32_t
syntax_var(program_t *program, node_t *node)
{
	//node_import_t *node_import = (node_import_t *)node->value;
	return 1;
}


static int32_t
syntax_module(program_t *program, node_t *node)
{
	node_module_t *node_module = (node_module_t *)node->value;
    
    if (node_module->block != NULL)
    {
        node_t *node1 = node_module->block;
        node_block_t *node_block1 = (node_block_t *)node1->value;
        ilist_t *a1;
        for (a1 = node_block1->list->begin; a1 != node_block1->list->end; a1 = a1->next)
        {
            node_t *item = (node_t *)a1->value;
            if (item->kind == NODE_KIND_IMPORT)
            {
                int32_t result;
                result = syntax_import(program, item);
                if (result == -1)
                {
                    return -1;
                }
            }
            else
            if (item->kind == NODE_KIND_CLASS)
            {
                int32_t result;
                result = syntax_class(program, item);
                if (result == -1)
                {
                    return -1;
                }
            }
            else
            if (item->kind == NODE_KIND_ENUM)
            {
                int32_t result;
                result = syntax_enum(program, item);
                if (result == -1)
                {
                    return -1;
                }
            }
            else
            if (item->kind == NODE_KIND_FUNC)
            {
                int32_t result;
                result = syntax_func(program, item);
                if (result == -1)
                {
                    return -1;
                }
            }
            else
            if (item->kind == NODE_KIND_VAR)
            {
                int32_t result;
                result = syntax_var(program, item);
                if (result == -1)
                {
                    return -1;
                }
            }
        }
    }

	return 1;
}

int32_t
syntax_run(program_t *program, node_t *node)
{
	int32_t result = syntax_module(program, node);
	if(result == -1)
	{
		return -1;
	}
	return 1;
}
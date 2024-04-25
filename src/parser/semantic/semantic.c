/*
 * Creator: Yasser Sajjadi(Ys)
 * Date: 24/3/2024
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "../../types/types.h"
#include "../../container/list.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../program.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../syntax/syntax.h"
#include "../error.h"
#include "semantic.h"

error_t *
semantic_error(program_t *program, node_t *node, const char *format, ...)
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

int32_t
semantic_idcmp(node_t *n1, node_t *n2)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	node_basic_t *nb2 = (node_basic_t *)n2->value;

	//printf("%s %s\n", nb1->value, nb2->value);

	return (strcmp(nb1->value, nb2->value) == 0);
}

int32_t
semantic_idstrcmp(node_t *n1, char *name)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}


static int32_t
semantic_var(program_t *program, node_t *node, uint64_t flag);

static int32_t
semantic_parameter(program_t *program, node_t *node, uint64_t flag);

static int32_t
semantic_parameters(program_t *program, node_t *node, uint64_t flag);

static int32_t
semantic_generics(program_t *program, node_t *node, uint64_t flag);

static int32_t
semantic_body(program_t *program, node_t *node, uint64_t flag);


static int32_t
semantic_if(program_t *program, node_t *node, uint64_t flag)
{
    node_if_t *if1 = (node_if_t *)node->value;

    if (if1->condition != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, if1->condition, response1, SEMANTIC_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        {
            uint64_t cnt_response1 = 0;

            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                cnt_response1 += 1;

                node_t *item1 = (node_t *)a1->value;

                if (item1->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)item1->value;
                    if ((item1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, if1->condition, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                        return -1;
                    }
                    continue;
                }
                else
                {
                    semantic_error(program, if1->condition, "Not an instance of object\n\tInternal:%s-%u",__FILE__, __LINE__);
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, if1->condition, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

    if (if1->then_body != NULL)
    {
        int32_t r1 = semantic_body(program, if1->then_body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (if1->else_body != NULL)
    {
        node_t *else_body1 = if1->else_body;

        if (else_body1->kind == NODE_KIND_IF)
        {
            int32_t r1 = semantic_if(program, if1->else_body, flag);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        {
            int32_t r1 = semantic_body(program, if1->else_body, flag);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
semantic_for(program_t *program, node_t *node, uint64_t flag)
{
    node_for_t *for1 = (node_for_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_CATCH)
            {
                node_catch_t *catch2 = (node_catch_t *)current->value;
                node_t *node2 = catch2->parameters;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                        if (semantic_idcmp(for1->key, parameter2->key) == 1)
                        {
                            semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                parameter2->key->position.line, parameter2->key->position.column);
                            return -1;
                        }
                    }
                }
                sub = current;
                current = current->parent;
                continue;
            }
            else
            if (current->kind == NODE_KIND_FOR)
            {
                node_for_t *for2 = (node_for_t *)current->value;
                node_t *node2 = for2->initializer;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current->kind == NODE_KIND_FORIN)
            {
                node_forin_t *for2 = (node_forin_t *)current->value;
                node_t *node2 = for2->initializer;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current->kind == NODE_KIND_BODY)
            {
                node_block_t *block2 = (node_block_t *)current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == sub->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *for2 = (node_forin_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                
                sub = current;
                current = current->parent;
            }
            else
            if (current->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun2 = (node_fun_t *)current->value;

                if (fun2->generics != NULL)
                {
                    node_t *node2 = fun2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(for1->key, generic3->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    generic3->key->position.line, generic3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }

                if (fun2->parameters != NULL)
                {
                    node_t *node3 = fun2->parameters;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        
                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (semantic_idcmp(for1->key, parameter3->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    parameter3->key->position.line, parameter3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                sub = current;
                current = current->parent;
                continue;
            }
        }
    }
    
    if (for1->initializer != NULL)
    {
        node_t *node1 = for1->initializer;
        node_block_t *block1 = node1->value;

        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_VAR)
            {
                int32_t r1 = semantic_var(program, item1, SEMANTIC_FLAG_NONE);
                if (r1 == -1)
                {
                    return -1;
                }
            }
            else
            {
                int32_t r1 = semantic_assign(program, item1, SEMANTIC_FLAG_NONE);
                if (r1 == -1)
                {
                    return -1;
                }
            }
        }
    }

    if (for1->condition != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, for1->condition, response1, SEMANTIC_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        {
            uint64_t cnt_response1 = 0;

            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                cnt_response1 += 1;

                node_t *item1 = (node_t *)a1->value;

                if (item1->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)item1->value;
                    if ((item1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, for1->condition, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                        return -1;
                    }
                    continue;
                }
                else
                {
                    semantic_error(program, for1->condition, "Not an instance of object\n\tInternal:%s-%u",__FILE__, __LINE__);
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, for1->condition, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

    if (for1->incrementor != NULL)
    {
        node_t *node1 = for1->incrementor;
        node_block_t *block1 = node1->value;

        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            int32_t r1 = semantic_assign(program, item1, SEMANTIC_FLAG_NONE);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }

    if (for1->body != NULL)
    {
        int32_t r1 = semantic_body(program, for1->body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_forin(program_t *program, node_t *node, uint64_t flag)
{
    node_forin_t *for1 = (node_forin_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_CATCH)
            {
                node_catch_t *catch2 = (node_catch_t *)current->value;
                node_t *node2 = catch2->parameters;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                        if (semantic_idcmp(for1->key, parameter2->key) == 1)
                        {
                            semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                parameter2->key->position.line, parameter2->key->position.column);
                            return -1;
                        }
                    }
                }
                sub = current;
                current = current->parent;
                continue;
            }
            else
            if (current->kind == NODE_KIND_FOR)
            {
                node_for_t *for2 = (node_for_t *)current->value;
                node_t *node2 = for2->initializer;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current->kind == NODE_KIND_FORIN)
            {
                node_forin_t *for2 = (node_forin_t *)current->value;
                node_t *node2 = for2->initializer;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current->kind == NODE_KIND_BODY)
            {
                node_block_t *block2 = (node_block_t *)current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == sub->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *for2 = (node_forin_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                
                sub = current;
                current = current->parent;
            }
            else
            if (current->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun2 = (node_fun_t *)current->value;

                if (fun2->generics != NULL)
                {
                    node_t *node2 = fun2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(for1->key, generic3->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    generic3->key->position.line, generic3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }

                if (fun2->parameters != NULL)
                {
                    node_t *node3 = fun2->parameters;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        
                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (semantic_idcmp(for1->key, parameter3->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    parameter3->key->position.line, parameter3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                sub = current;
                current = current->parent;
                continue;
            }
        }
    }

    if (for1->initializer != NULL)
    {
        node_t *node1 = for1->initializer;
        node_block_t *block1 = node1->value;

        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_VAR)
            {
                int32_t r1 = semantic_var(program, item1, SEMANTIC_FLAG_NONE);
                if (r1 == -1)
                {
                    return -1;
                }
            }
            else
            {
                int32_t r1 = semantic_assign(program, item1, SEMANTIC_FLAG_NONE);
                if (r1 == -1)
                {
                    return -1;
                }
            }
        }
    }

    if (for1->expression != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, for1->expression, response1, SEMANTIC_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        {
            uint64_t cnt_response1 = 0;

            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                cnt_response1 += 1;

                node_t *item1 = (node_t *)a1->value;

                if (item1->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)item1->value;
                    if ((item1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, for1->expression, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                        return -1;
                    }
                    continue;
                }
                else
                {
                    semantic_error(program, for1->expression, "Not an instance of object\n\tInternal:%s-%u",__FILE__, __LINE__);
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, for1->expression, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

    if (for1->body != NULL)
    {
        int32_t r1 = semantic_body(program, for1->body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
semantic_parameter(program_t *program, node_t *node, uint64_t flag)
{
    node_parameter_t *parameter1 = (node_parameter_t *)node->value;
    
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_PARAMETERS)
            {
                node_block_t *block2 = (node_block_t *)current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == sub->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter3 = (node_parameter_t *)item2->value;
                        if (semantic_idcmp(parameter1->key, parameter3->key) == 1)
                        {
                            semantic_error(program, parameter1->key, "already defined, previous in (%lld:%lld)",
                                parameter3->key->position.line, parameter3->key->position.column);
                            return -1;
                        }
                    }
                }
                
                sub = current;
                current = current->parent;
                continue;
            }
            else
            if (current->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        
                        if (item3->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item3->value;
                            if (semantic_idcmp(parameter1->key, generic3->key) == 1)
                            {
                                semantic_error(program, parameter1->key, "already defined, previous in (%lld:%lld)",
                                    generic3->key->position.line, generic3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun2 = (node_fun_t *)current->value;

                if (fun2->generics != NULL)
                {
                    node_t *node2 = fun2->generics;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        
                        if (item3->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item3->value;
                            if (semantic_idcmp(parameter1->key, generic3->key) == 1)
                            {
                                semantic_error(program, parameter1->key, "already defined, previous in (%lld:%lld)",
                                    generic3->key->position.line, generic3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                sub = current;
                current = current->parent;
                continue;
            }
        }
    }

    if (parameter1->value != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, parameter1->value, response1, SEMANTIC_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        {
            uint64_t cnt_response1 = 0;

            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                cnt_response1 += 1;

                node_t *item1 = (node_t *)a1->value;

                int32_t r2 = semantic_assignment(program, node, item1, node, SEMANTIC_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, parameter1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
    }

    return 1;
}

static int32_t
semantic_parameters(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *parameters = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = parameters->list->begin;a1 != parameters->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_parameter(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_generic(program_t *program, node_t *node, uint64_t flag)
{
    node_generic_t *generic1 = (node_generic_t *)node->value;
    
    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_GENERICS)
        {
            node_block_t *block2 = (node_block_t *)current->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic3 = (node_generic_t *)item2->value;
                    if (semantic_idcmp(generic1->key, generic3->key) == 1)
                    {
                        semantic_error(program, generic1->key, "already defined, previous in (%lld:%lld)",
                            generic3->key->position.line, generic3->key->position.column);
                        return -1;
                    }
                }
            }
            
            break;
        }
        else
        {
            sub = current;
            current = current->parent;
            continue;
        }
    }

    return 1;
}

static int32_t
semantic_generics(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *generics = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = generics->list->begin;a1 != generics->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_generic(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_catch(program_t *program, node_t *node, uint64_t flag)
{
    node_catch_t *catch1 = (node_catch_t *)node->value;

    if (catch1->parameters != NULL)
    {
        int32_t r1 = semantic_parameters(program, catch1->parameters, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (catch1->body != NULL)
    {
        int32_t r1 = semantic_body(program, catch1->body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
semantic_try(program_t *program, node_t *node, uint64_t flag)
{
	node_try_t *try1 = (node_try_t *)node->value;

    if (try1->body != NULL)
    {
        int32_t r1 = semantic_body(program, try1->body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    node_t *node2 = try1->catchs;
    node_block_t *block2 = (node_block_t *)node2->value;

    ilist_t *a1;
    for (a1 = block2->list->begin;a1 != block2->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result = semantic_catch(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_var(program_t *program, node_t *node, uint64_t flag)
{
    node_var_t *var1 = (node_var_t *)node->value;

    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_CATCH)
        {
            node_catch_t *catch2 = (node_catch_t *)current->value;
            node_t *node2 = catch2->parameters;
            node_block_t *block2 = node2->value;
            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (semantic_idcmp(var1->key, parameter2->key) == 1)
                        {
                            semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                parameter2->key->position.line, parameter2->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        node_t *node4 = (node_t *)var1->key;
                        node_block_t *block4 = (node_block_t *)node4->value;
                        ilist_t *a4;
                        for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                        {
                            node_t *item4 = (node_t *)a4->value;
                            if (item4->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity4 = (node_entity_t *)item4->value;
                                if (semantic_idcmp(entity4->key, parameter2->key) == 1)
                                {
                                    semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                        parameter2->key->position.line, parameter2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
            }
            sub = current;
            current = current->parent;
            continue;
        }
        else
        if (current->kind == NODE_KIND_FOR)
        {
            node_for_t *for2 = (node_for_t *)current->value;
            node_t *node2 = for2->initializer;
            node_block_t *block2 = node2->value;
            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node4 = var1->key;
                            node_block_t *block4 = (node_block_t *)node4->value;
                            ilist_t *a4;
                            for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                            {
                                node_t *item4 = (node_t *)a4->value;
                                if (item4->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity4 = (node_entity_t *)item4->value;
                                    if (semantic_idcmp(entity4->key, var2->key) == 1)
                                    {
                                        semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                            var2->key->position.line, var2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        node_t *node3 = var2->key;
                        node_block_t *block3 = (node_block_t *)node3->value;
                        ilist_t *a3;
                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;
                            if (item3->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity2 = (node_entity_t *)item3->value;
                                if (var1->key->kind == NODE_KIND_ID)
                                {
                                    if (semantic_idcmp(var1->key, entity2->key) == 1)
                                    {
                                        semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                            entity2->key->position.line, entity2->key->position.column);
                                        return -1;
                                    }
                                }
                                else
                                {
                                    node_t *node4 = (node_t *)var1->key;
                                    node_block_t *block4 = (node_block_t *)node4->value;
                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_ENTITY)
                                        {
                                            node_entity_t *entity4 = (node_entity_t *)item4->value;
                                            if (semantic_idcmp(entity4->key, entity2->key) == 1)
                                            {
                                                semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                                    entity2->key->position.line, entity2->key->position.column);
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
            break;
        }
        else
        if (current->kind == NODE_KIND_FORIN)
        {
            node_forin_t *for2 = (node_forin_t *)current->value;
            node_t *node2 = for2->initializer;
            node_block_t *block2 = node2->value;
            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node4 = (node_t *)var1->key;
                            node_block_t *block4 = (node_block_t *)node4->value;
                            ilist_t *a4;
                            for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                            {
                                node_t *item4 = (node_t *)a4->value;
                                if (item4->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity4 = (node_entity_t *)item4->value;
                                    if (semantic_idcmp(entity4->key, var2->key) == 1)
                                    {
                                        semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                            var2->key->position.line, var2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        node_t *node3 = var2->key;
                        node_block_t *block3 = (node_block_t *)node3->value;
                        ilist_t *a3;
                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;
                            if (item3->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity2 = (node_entity_t *)item3->value;
                                if (var1->key->kind == NODE_KIND_ID)
                                {
                                    if (semantic_idcmp(var1->key, entity2->key) == 1)
                                    {
                                        semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                            entity2->key->position.line, entity2->key->position.column);
                                        return -1;
                                    }
                                }
                                else
                                {
                                    node_t *node4 = (node_t *)var1->key;
                                    node_block_t *block4 = (node_block_t *)node4->value;
                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_ENTITY)
                                        {
                                            node_entity_t *entity4 = (node_entity_t *)item4->value;
                                            if (semantic_idcmp(entity4->key, entity2->key) == 1)
                                            {
                                                semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                                    entity2->key->position.line, entity2->key->position.column);
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
            break;
        }
        else
        if (current->kind == NODE_KIND_BODY)
        {
            node_block_t *block2 = (node_block_t *)current->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for2 = (node_for_t *)item2->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, for2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(entity1->key, for2->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            for2->key->position.line, for2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FORIN)
                {
                    node_forin_t *for2 = (node_forin_t *)item2->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, for2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(entity1->key, for2->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            for2->key->position.line, for2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(entity1->key, var2->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            var2->key->position.line, var2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            node_t *node3 = (node_t *)var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(var1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *enteiry1 = (node_entity_t *)item3->value;
                                    node_t *node4 = (node_t *)var2->key;
                                    node_block_t *block4 = (node_block_t *)node4->value;
                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_ENTITY)
                                        {
                                            node_entity_t *enteiry2 = (node_entity_t *)item4->value;
                                            if (semantic_idcmp(enteiry1->key, enteiry2->key) == 1)
                                            {
                                                semantic_error(program, enteiry1->key, "already defined, previous in (%lld:%lld)",
                                                    enteiry2->key->position.line, enteiry2->key->position.column);
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
            sub = current;
            current = current->parent;
            continue;
        }
        else
        if (current->kind == NODE_KIND_FUN)
        {
            node_fun_t *fun2 = (node_fun_t *)current->value;

            if (fun2->generics != NULL)
            {
                node_t *node2 = fun2->generics;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item2->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item2->value;
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, generic3->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    generic3->key->position.line, generic3->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(entity1->key, generic3->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            generic3->key->position.line, generic3->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (fun2->parameters != NULL)
            {
                node_t *node3 = fun2->parameters;
                node_block_t *block3 = (node_block_t *)node3->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    
                    if (item3->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, parameter3->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    parameter3->key->position.line, parameter3->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node4 = (node_t *)var1->key;
                            node_block_t *block4 = (node_block_t *)node4->value;
                            ilist_t *a4;
                            for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                            {
                                node_t *item4 = (node_t *)a4->value;
                                if (item4->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item4->value;
                                    if (semantic_idcmp(entity1->key, parameter3->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            parameter3->key->position.line, parameter3->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            break;
        }
        else
        {
            sub = current;
            current = current->parent;
            continue;
        }
    }

    if (var1->value != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, var1->value, response1, SEMANTIC_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        {
            uint64_t cnt_response1 = 0;

            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                cnt_response1 += 1;

                node_t *item1 = (node_t *)a1->value;

                int32_t r2 = semantic_assignment(program, node, item1, node, SEMANTIC_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, var1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

	return 1;
}

static int32_t
semantic_return(program_t *program, node_t *node, uint64_t flag)
{
    node_unary_t *unary1 = (node_unary_t *)node->value;

    if (unary1->right != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, unary1->right, response1, SEMANTIC_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        {
            uint64_t cnt_response1 = 0;

            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                cnt_response1 += 1;

                node_t *item1 = (node_t *)a1->value;

                if (item1->kind == NODE_KIND_LAMBDA)
                {
                    continue;
                }
                else
                if (item1->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)node->value;
                    if ((item1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, unary1->right, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                        return -1;
                    }
                    continue;
                }
                else
                {
                    semantic_error(program, unary1->right, "Not an instance of object or fun\n\tInternal:%s-%u",__FILE__, __LINE__);
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, unary1->right, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

    return 1;
}

static int32_t
semantic_continue(program_t *program, node_t *node, uint64_t flag)
{
    return 1;
}

static int32_t
semantic_break(program_t *program, node_t *node, uint64_t flag)
{
    return 1;
}

static int32_t
semantic_statement(program_t *program, node_t *node, uint64_t flag)
{
    if (node->kind == NODE_KIND_IF)
    {
        int32_t result = semantic_if(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else 
    if (node->kind == NODE_KIND_FOR)
    {
        int32_t result = semantic_for(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_FORIN)
    {
        int32_t result = semantic_forin(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_TRY)
    {
        int32_t result = semantic_try(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t result = semantic_var(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_RETURN)
    {
        int32_t result = semantic_return(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_CONTINUE)
    {
        int32_t result = semantic_continue(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_BREAK)
    {
        int32_t result = semantic_break(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    {
        return semantic_assign(program, node, flag);
    }
    return 1;
}

static int32_t
semantic_body(program_t *program, node_t *node, uint64_t flag)
{
    node_block_t *block1 = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        int32_t result = semantic_statement(program, item, flag);
        if (result == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
semantic_fun(program_t *program, node_t *node, uint64_t flag)
{
	node_fun_t *fun1 = (node_fun_t *)node->value;

    {
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(fun1->key, generic1->key) == 1)
                            {
                                semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    generic1->key->position.line, generic1->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                
                if (class2->heritages != NULL)
                {
                    node_t *node2 = class2->heritages;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage1 = (node_heritage_t *)item2->value;

                            if (semantic_idcmp(fun1->key, heritage1->key) == 1)
                            {
                                semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    heritage1->key->position.line, heritage1->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                
                node_t *node3 = class2->block;
                node_block_t *block3 = (node_block_t *)node3->value;

                ilist_t *a2;
                for (a2 = block3->list->begin;a2 != block3->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item2->value;
                        if (semantic_idcmp(fun1->key, class3->key) == 1)
                        {
                            node_t *ngs1 = class3->generics;
                            node_t *ngs2 = fun1->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun2 = (node_fun_t *)item2->value;

                        if (semantic_idcmp(fun1->key, fun2->key) == 1)
                        {
                            node_t *ngs1 = fun1->generics;
                            node_t *ngs2 = fun2->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                node_t *nps1 = fun1->parameters;
                                node_t *nps2 = fun2->parameters;
                                int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                        fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_PROPERTY)
                    {
                        node_property_t *property2 = (node_property_t *)item2->value;

                        if (semantic_idcmp(fun1->key, property2->key) == 1)
                        {
                            semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            {
                current = current->parent;
                continue;
            }
        }
    }
    
    if (fun1->annotation != NULL)
    {
        node_t *node1 = fun1->annotation;
        while (node1 != NULL)
        {
            node_note_t *annotation1 = (node_note_t *)node1->value;
            node_t *node2 = fun1->annotation;
            while (node2 != NULL)
            {
                if (node1->id == node2->id)
                {
                    break;
                }
                node_note_t *annotation2 = (node_note_t *)node2->value;
                if (semantic_idcmp(annotation1->key, annotation2->key) == 1)
                {
                    semantic_error(program, annotation1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u", 
                        annotation2->key->position.line, annotation2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
                node2 = annotation2->next;
            }

            if (semantic_idstrcmp(annotation1->key, "Main") == 1)
            {
                if (program->main == NULL)
                {
                    if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                    {
                        semantic_error(program, fun1->key, "Non-Static, 'Main' must be a static function\n\tInternal:%s-%u", 
                            __FILE__, __LINE__);
                        return -1;
                    }
                    program->main = node;
                }
                else
                {
                    node_t *node2 = program->main;
                    node_fun_t *fun2 = (node_fun_t *)node2->value;

                    semantic_error(program, fun1->key, "'Main' is already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                        fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            node1 = annotation1->next;
        }
    }

    if (fun1->generics != NULL)
    {
        int32_t r1 = semantic_generics(program, fun1->generics, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->parameters != NULL)
    {
        int32_t r1 = semantic_parameters(program, fun1->parameters, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->body != NULL)
    {
        int32_t r1 = semantic_body(program, fun1->body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_property(program_t *program, node_t *node, uint64_t flag)
{
    node_property_t *property1 = (node_property_t *)node->value;
    
    {
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;

                        if (item3->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item3->value;
                            if (semantic_idcmp(property1->key, generic3->key) == 1)
                            {
                                semantic_error(program, property1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    generic3->key->position.line, generic3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }

                if (class2->heritages != NULL)
                {
                    node_t *node2 = class2->heritages;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;

                        if (item3->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage3 = (node_heritage_t *)item3->value;
                            if (semantic_idcmp(property1->key, heritage3->key) == 1)
                            {
                                semantic_error(program, property1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    heritage3->key->position.line, heritage3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }

                node_t *node2 = class2->block;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item2->value;
                        if (semantic_idcmp(property1->key, class3->key) == 1)
                        {
                            semantic_error(program, property1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun2 = (node_fun_t *)item2->value;

                        if (semantic_idcmp(property1->key, fun2->key) == 1)
                        {
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_PROPERTY)
                    {
                        node_property_t *property2 = (node_property_t *)item2->value;

                        if (semantic_idcmp(property1->key, property2->key) == 1)
                        {
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            {
                current = current->parent;
                continue;
            }
        }
    }

    if (property1->annotation != NULL)
    {
        node_t *node1 = property1->annotation;
        while (node1 != NULL)
        {
            node_note_t *annotation1 = (node_note_t *)node1->value;
            node_t *node2 = property1->annotation;
            while (node2 != NULL)
            {
                if (node1->id == node2->id)
                {
                    break;
                }
                node_note_t *annotation2 = (node_note_t *)node2->value;
                if (semantic_idcmp(annotation1->key, annotation2->key) == 1)
                {
                    semantic_error(program, annotation1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u", 
                        annotation2->key->position.line, annotation2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
                node2 = annotation2->next;
            }

            node1 = annotation1->next;
        }
    }

    if (property1->value != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, property1->value, response1, SEMANTIC_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        {
            uint64_t cnt_response1 = 0;

            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                cnt_response1 += 1;

                node_t *item1 = (node_t *)a1->value;

                int32_t r2 = semantic_assignment(program, node, item1, node, SEMANTIC_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, property1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
    }

    return 1;
}

static int32_t
semantic_heritage(program_t *program, node_t *node, uint64_t flag)
{
    node_heritage_t *heritage1 = (node_heritage_t *)node->value;
    
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_HERITAGES)
            {
                node_block_t *block2 = (node_block_t *)current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == sub->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_HERITAGE)
                    {
                        node_heritage_t *heritage3 = (node_heritage_t *)item2->value;
                        if (semantic_idcmp(heritage1->key, heritage3->key) == 1)
                        {
                            semantic_error(program, heritage1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                heritage3->key->position.line, heritage3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                
                sub = current;
                current = current->parent;
                continue;
            }
            else
            if (current->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;

                        if (item3->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item3->value;
                            if (semantic_idcmp(heritage1->key, generic3->key) == 1)
                            {
                                semantic_error(program, heritage1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    generic3->key->position.line, generic3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                sub = current;
                current = current->parent;
                continue;
            }
        }
    }

    return 1;
}

static int32_t
semantic_heritages(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *heritages = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = heritages->list->begin;a1 != heritages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_heritage(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_class(program_t *program, node_t *node, uint64_t flag)
{
	node_class_t *class1 = (node_class_t *)node->value;

    {
        node_t *sub1 = node;
        node_t *current1 = node->parent;
        while (current1 != NULL)
        {
            if (current1->kind == NODE_KIND_MODULE)
            {
                node_module_t *module2 = (node_module_t *)current1->value;

                ilist_t *a1;
                for (a1 = module2->items->begin;a1 != module2->items->end;a1 = a1->next)
                {
                    node_t *item1 = (node_t *)a1->value;
                    if (item1->id == sub1->id)
                    {
                        break;
                    }
                    
                    if (item1->kind == NODE_KIND_USING)
                    {
                        node_using_t *usage1 = (node_using_t *)item1->value;

                        if (usage1->packages != NULL)
                        {
                            node_t *node1 = usage1->packages;
                            node_block_t *block1 = (node_block_t *)node1->value;
                            ilist_t *a2;
                            for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_PACKAGE)
                                {
                                    node_package_t *package1 = (node_package_t *)item2->value;
                                    if (semantic_idcmp(package1->key, class1->key) == 1)
                                    {
                                        semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                            package1->key->position.line, package1->key->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            node_basic_t *basic2 = (node_basic_t *)usage1->path->value;

                            node_t *node1 = program_load(program, basic2->value);
                            if (node1 == NULL)
                            {
                                return -1;
                            }
                            node_module_t *module1 = (node_module_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = module1->items->begin; a2 != module1->items->end; a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    node_class_t *class2 = (node_class_t *)item2->value;
                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                                    {
                                        if (semantic_idcmp(class2->key, class1->key) == 1)
                                        {
                                            int32_t r1 = semantic_eqaul_gsgs(program, class2->generics, class1->generics);
                                            if (r1 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r1 == 1)
                                            {
                                                semantic_error(program, class1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                    class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item1->value;
                        if (semantic_idcmp(class1->key, class3->key) == 1)
                        {
                            node_t *ngs1 = class1->generics;
                            node_t *ngs2 = class3->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current1->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(class1->key, generic1->key) == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    generic1->key->position.line, generic1->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                
                if (class2->heritages != NULL)
                {
                    node_t *node2 = class2->heritages;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage1 = (node_heritage_t *)item2->value;

                            if (semantic_idcmp(class1->key, heritage1->key) == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    heritage1->key->position.line, heritage1->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                
                node_t *node2 = class2->block;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item2->value;
                        if (semantic_idcmp(class1->key, class3->key) == 1)
                        {
                            node_t *ngs1 = class1->generics;
                            node_t *ngs2 = class3->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun2 = (node_fun_t *)item2->value;

                        if (semantic_idcmp(class1->key, fun2->key) == 1)
                        {
                            node_t *ngs1 = class1->generics;
                            node_t *ngs2 = fun2->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_PROPERTY)
                    {
                        node_property_t *property2 = (node_property_t *)item2->value;

                        if (semantic_idcmp(class1->key, property2->key) == 1)
                        {
                            semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            {
                sub1 = current1;
                current1 = current1->parent;
                continue;
            }
        }
    }

    if (class1->annotation != NULL)
    {
        node_t *node1 = class1->annotation;
        while (node1 != NULL)
        {
            node_note_t *annotation1 = (node_note_t *)node1->value;
            node_t *node2 = class1->annotation;
            while (node2 != NULL)
            {
                if (node1->id == node2->id)
                {
                    break;
                }
                node_note_t *annotation2 = (node_note_t *)node2->value;
                if (semantic_idcmp(annotation1->key, annotation2->key) == 1)
                {
                    semantic_error(program, annotation1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u", 
                        annotation2->key->position.line, annotation2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
                node2 = annotation2->next;
            }

            node1 = annotation1->next;
        }
    }

    if (class1->generics != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            semantic_error(program, class1->key, "Generic type in static class\n\tInternal:%s-%u", 
                __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_generics(program, class1->generics, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->heritages != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            semantic_error(program, class1->key, "Generic type in inheritance\n\tInternal:%s-%u", 
                __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_heritages(program, class1->heritages, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    node_t *node1 = class1->block;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        
        if (item1->kind == NODE_KIND_CLASS)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                node_class_t *class2 = (node_class_t *)item1->value;
                if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    semantic_error(program, class2->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            node_t *clone1 = node_clone(item1->parent, item1);
            if (clone1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }

            clone1->flag |= NODE_FLAG_INSTANCE;

            int32_t result = semantic_class(program, clone1, flag);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item1->kind == NODE_KIND_FUN)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;
                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    semantic_error(program, fun1->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            int32_t result = semantic_fun(program, item1, flag);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item1->kind == NODE_KIND_PROPERTY)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                node_property_t *property1 = (node_property_t *)item1->value;
                if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    semantic_error(program, property1->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            int32_t result = semantic_property(program, item1, flag);
            if (result == -1)
            {
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
semantic_package(program_t *program, node_t *node, uint64_t flag)
{
    node_package_t *package1 = (node_package_t *)node->value;

    if (package1->address != NULL)
    {
        node_t *address1 = package1->address;
        while(address1 != NULL)
        {
            if (address1->kind == NODE_KIND_PSEUDONYM)
            {
                node_carrier_t *carrier1 = (node_carrier_t *)address1->value;
                address1 = carrier1->base;
            }
            else
            if (address1->kind == NODE_KIND_ATTRIBUTE)
            {
                node_binary_t *binary = (node_binary_t *)address1->value;
                address1 = binary->left;
            }
            else
            if (address1->kind == NODE_KIND_ID)
            {
                break;
            }
            else
            {
                semantic_error(program, package1->address, "Not a address\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }  
    }   

    {
        node_t *sub1 = node;
        node_t *current1 = node->parent;
        while (current1 != NULL)
        {
            if (current1->kind == NODE_KIND_PACKAGES)
            {
                node_block_t *block3 = (node_block_t *)current1->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    if (item3->kind == NODE_KIND_PACKAGE)
                    {
                        if (item3->id == node->id)
                        {
                            break;
                        }

                        node_package_t *package3 = (node_package_t *)item3->value;

                        if (semantic_idcmp(package1->key, package3->key) == 1)
                        {
                            semantic_error(program, package1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                package3->key->position.line, package3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }

                sub1 = current1;
                current1 = current1->parent;
                continue;
            }
            else
            if (current1->kind == NODE_KIND_MODULE)
            {
                node_module_t *module1 = (node_module_t *)current1->value;

                ilist_t *a1;
                for (a1 = module1->items->begin;a1 != module1->items->end;a1 = a1->next)
                {
                    node_t *item1 = (node_t *)a1->value;
                    
                    if (item1->id == sub1->id)
                    {
                        break;
                    }

                    if (item1->kind == NODE_KIND_USING)
                    {
                        node_using_t *usage1 = (node_using_t *)item1->value;

                        if (usage1->packages != NULL)
                        {
                            node_t *node3 = usage1->packages;
                            node_block_t *block3 = (node_block_t *)node3->value;

                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_PACKAGE)
                                {
                                    node_package_t *package3 = (node_package_t *)item3->value;
                                    if (semantic_idcmp(package1->key, package3->key) == 1)
                                    {
                                        semantic_error(program, package1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                            package3->key->position.line, package3->key->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            node_basic_t *basic2 = (node_basic_t *)usage1->path->value;

                            node_t *node1 = program_load(program, basic2->value);
                            if (node1 == NULL)
                            {
                                return -1;
                            }
                            node_module_t *module1 = (node_module_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = module1->items->begin; a2 != module1->items->end; a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    node_class_t *class1 = (node_class_t *)item2->value;
                                    if ((class1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                                    {
                                        if (semantic_idcmp(class1->key, package1->key) == 1)
                                        {
                                            int32_t r1 = semantic_eqaul_gsgs(program, class1->generics, package1->generics);
                                            if (r1 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r1 == 1)
                                            {
                                                semantic_error(program, package1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                    class1->key->position.path, class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item1->value;

                        if (semantic_idcmp(package1->key, class3->key) == 1)
                        {
                            semantic_error(program, package1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            {
                sub1 = current1;
                current1 = current1->parent;
                continue;
            }
        }
    }

    if (package1->generics != NULL)
    {
        int32_t r1 = semantic_generics(program, package1->generics, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
semantic_packages(program_t *program, node_t *node, uint64_t flag)
{
    node_block_t *packages = (node_block_t *)node->value;
    
	ilist_t *a1;
    for (a1 = packages->list->begin;a1 != packages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_package(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_using(program_t *program, node_t *node, uint64_t flag)
{
	node_using_t *usage1 = (node_using_t *)node->value;

	if (usage1->packages != NULL)
    {
        int32_t r1 = semantic_packages(program, usage1->packages, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }
    else
    {
        node_basic_t *basic2 = (node_basic_t *)usage1->path->value;

        node_t *node1 = program_load(program, basic2->value);
        if (node1 == NULL)
        {
            return -1;
        }

        node_module_t *module1 = (node_module_t *)node1->value;

        ilist_t *a1;
        for (a1 = module1->items->begin; a1 != module1->items->end; a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)item1->value;
                if ((class1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                {
                    node_t *sub1 = node;
                    node_t *current1 = node->parent;
                    while (current1 != NULL)
                    {
                        if (current1->kind == NODE_KIND_MODULE)
                        {
                            node_module_t *module2 = (node_module_t *)current1->value;

                            ilist_t *a2;
                            for (a2 = module2->items->begin;a2 != module2->items->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                
                                if (item2->id == sub1->id)
                                {
                                    break;
                                }

                                if (item2->kind == NODE_KIND_USING)
                                {
                                    node_using_t *usage2 = (node_using_t *)item2->value;
                                    node_t *node2 = usage2->packages;
                                    node_block_t *block1 = (node_block_t *)node2->value;

                                    ilist_t *a3;
                                    for (a3 = block1->list->begin;a3 != block1->list->end;a3 = a3->next)
                                    {
                                        node_t *item3 = (node_t *)a3->value;
                                        if (item3->kind == NODE_KIND_PACKAGE)
                                        {
                                            node_package_t *package1 = (node_package_t *)item3->value;
                                            if (semantic_idcmp(class1->key, package1->key) == 1)
                                            {
                                                int32_t r1 = semantic_eqaul_gsgs(program, class1->generics, package1->generics);
                                                if (r1 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r1 == 1)
                                                {
                                                    semantic_error(program, class1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                        package1->key->position.path, package1->key->position.line, package1->key->position.column, __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    node_class_t *class2 = (node_class_t *)item2->value;

                                    if (semantic_idcmp(class1->key, class2->key) == 1)
                                    {
                                        int32_t r1 = semantic_eqaul_gsgs(program, class1->generics, class2->generics);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 1)
                                        {
                                            semantic_error(program, class1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        else
                        {
                            sub1 = current1;
                            current1 = current1->parent;
                            continue;
                        }
                    }
                }
            }
        }
    }

	return 1;
}

int32_t
semantic_module(program_t *program, node_t *node)
{
	node_module_t *module = (node_module_t *)node->value;

    ilist_t *a1;
    for (a1 = module->items->begin; a1 != module->items->end; a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;

        if (item1->kind == NODE_KIND_USING)
        {
            int32_t result = semantic_using(program, item1, SEMANTIC_FLAG_NONE);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item1->kind == NODE_KIND_CLASS)
        {
            node_t *clone1 = node_clone(item1->parent, item1);
            if (clone1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }

            clone1->flag |= NODE_FLAG_INSTANCE;

            int32_t result = semantic_class(program, clone1, SEMANTIC_FLAG_NONE);
            if (result == -1)
            {
                return -1;
            }
        }
    }
    
	return 1;
}

int32_t
semantic_run(program_t *program, node_t *node)
{
    node_module_t *module1 = (node_module_t *)node->value;
    
    FILE *fout = fopen(program->out_file, "wb");
	if (fout == NULL)
	{
        fprintf(stderr, "could not create(%s)\n", program->out_file);
        return -1;
    }
    program->out = fout;

    int32_t found1 = 0, found2 = 0, found3 = 0;
    ilist_t *a1;
    for (a1 = module1->items->begin; a1 != module1->items->end; a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)item1->value;
            if (semantic_idstrcmp(class1->key, "System") == 1)
            {
                found1 = 1;

                node_t *node1 = class1->block;
                node_block_t *block1 = (node_block_t *)node1->value;
                
                ilist_t *a2;
                for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun1 = (node_fun_t *)item2->value;
                        if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                        {
                            found2 = 1;
                            fputs("global	_start\n\t", program->out);
                            fputs("section .text\n\t", program->out);
                            fputs("_start:\n", program->out);
                            /* int32_t r1 = semantic_fun(program, item2);
                            if (r1 == -1)
                            {
                                return -1;
                            } */
                        }
                    }
                }

                if (found2 == 0)
                {
                    semantic_error(program, item1, "'Constructor' not found");
                    fclose(program->out);
                    return -1;
                }

                if (found3 == 0)
                {
                    semantic_error(program, item1, "'Constructor' missmaching");
                    fclose(program->out);
                    return -1;
                }
            }
        }
    }

    if (found1 == 0)
    {
        semantic_error(program, node, "'System' class not found");
        fclose(program->out);
        return -1;
    }

    fclose(program->out);
	return 1;
}
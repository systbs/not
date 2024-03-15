#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../container/table.h"
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

	return (strcmp(nb1->value, nb2->value) == 0);
}

static int32_t
syntax_idstrcmp(node_t *n1, char *name)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}

static int32_t
syntax_objidcmp(node_t *n1, node_t *n2)
{
    if (n1->kind == NODE_KIND_OBJECT)
    {
        node_object_t *node_object = (node_object_t *)n1->value;
        ilist_t *a1;
        for (a1 = node_object->list->begin;a1 != node_object->list->end;a1 = a1->next)
        {
            node_t *item = (node_t *)a1->value;
            if (item->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *node_property = (node_property_t *)item->value;
                if (node_property->type != NULL)
                {
                    int32_t r1 = syntax_objidcmp(node_property->type, n2);
                    if (r1 == 1)
                    {
                        return 1;
                    }
                }
                else
                {
                    int32_t r1 = syntax_objidcmp(node_property->key, n2);
                    if (r1 == 1)
                    {
                        return 1;
                    }
                }
            }
        }
        return 0;
    }
    else 
    if (n1->kind == NODE_KIND_ID)
    {
        if (n2->kind == NODE_KIND_OBJECT)
        {
            node_object_t *node_object = (node_object_t *)n2->value;
            ilist_t *a1;
            for (a1 = node_object->list->begin;a1 != node_object->list->end;a1 = a1->next)
            {
                node_t *item = (node_t *)a1->value;
                if (item->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *node_property = (node_property_t *)item->value;
                    if (node_property->type != NULL)
                    {
                        int32_t r1 = syntax_objidcmp(n1, node_property->type);
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                    else
                    {
                        int32_t r1 = syntax_objidcmp(n1, node_property->key);
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else 
        if (n2->kind == NODE_KIND_ID)
        {
            return (syntax_idcmp(n1, n2) == 1);
        }
    }
    return 0;
}


static int32_t
syntax_statement(program_t *program, node_t *node, list_t *frame);

static int32_t
syntax_body(program_t *program, node_t *node, list_t *frame);

static int32_t
syntax_expression(program_t *program, node_t *node, list_t *response);




static int32_t
syntax_expression(program_t *program, node_t *node, list_t *response)
{
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
syntax_if(program_t *program, node_t *node, list_t *frame)
{
    node_if_t *node_if1 = (node_if_t *)node->value;
    node_t *node1 = node_if1->then_body;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
    
        ilist_t *a2;
        for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;

            if (item1->id == item2->id)
            {
                break;
            }

            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(var1->key, var2->key) == 1)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_IF)
                {
                    node_if_t *if1 = (node_if_t *)item2->value;
                    if (if1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, if1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for1 = (node_for_t *)item2->value;
                    if (for1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, for1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FORIN)
                {
                    node_forin_t *forin1 = (node_forin_t *)item2->value;
                    if (forin1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, forin1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_IF)
            {
                node_if_t *if1 = (node_if_t *)item1->value;
                if (if1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(if1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FOR)
            {
                node_for_t *for1 = (node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(for1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FORIN)
            {
                node_forin_t *forin1 = (node_forin_t *)item1->value;
                if (forin1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(forin1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }

    if (node_if1->else_body != NULL)
    {
        if (node_if1->else_body->kind == NODE_KIND_BODY)
        {
            node1 = node_if1->else_body;
            block1 = (node_block_t *)node1->value;

            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
            
                ilist_t *a2;
                for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item1->id == item2->id)
                    {
                        break;
                    }

                    if (item1->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var1 = (node_var_t *)item1->value;
                        if (item2->kind == NODE_KIND_VAR)
                        {
                            node_var_t *var2 = (node_var_t *)item2->value;
                            if (syntax_objidcmp(var1->key, var2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_IF)
                        {
                            node_if_t *if1 = (node_if_t *)item2->value;
                            if (if1->key != NULL)
                            {
                                if (syntax_objidcmp(var1->key, if1->key) == 1)
                                {
                                    syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                        item2->position.line, item2->position.column);
                                    return -1;
                                }
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_FOR)
                        {
                            node_for_t *for1 = (node_for_t *)item2->value;
                            if (for1->key != NULL)
                            {
                                if (syntax_objidcmp(var1->key, for1->key) == 1)
                                {
                                    syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                        item2->position.line, item2->position.column);
                                    return -1;
                                }
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_FORIN)
                        {
                            node_forin_t *forin1 = (node_forin_t *)item2->value;
                            if (forin1->key != NULL)
                            {
                                if (syntax_objidcmp(var1->key, forin1->key) == 1)
                                {
                                    syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                        item2->position.line, item2->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_IF)
                    {
                        node_if_t *if1 = (node_if_t *)item1->value;
                        if (if1->key != NULL)
                        {
                            if (item2->kind == NODE_KIND_VAR)
                            {
                                node_var_t *var2 = (node_var_t *)item2->value;
                                if (syntax_objidcmp(if1->key, var2->key) == 1)
                                {
                                    syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                        item2->position.line, item2->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_IF)
                            {
                                node_if_t *if2 = (node_if_t *)item2->value;
                                if (if2->key != NULL)
                                {
                                    if (syntax_objidcmp(if1->key, if2->key) == 1)
                                    {
                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
                                        return -1;
                                    }
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_FOR)
                            {
                                node_for_t *for2 = (node_for_t *)item2->value;
                                if (for2->key != NULL)
                                {
                                    if (syntax_objidcmp(if1->key, for2->key) == 1)
                                    {
                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
                                        return -1;
                                    }
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_FORIN)
                            {
                                node_forin_t *forin2 = (node_forin_t *)item2->value;
                                if (forin2->key != NULL)
                                {
                                    if (syntax_objidcmp(if1->key, forin2->key) == 1)
                                    {
                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for1 = (node_for_t *)item1->value;
                        if (for1->key != NULL)
                        {
                            if (item2->kind == NODE_KIND_VAR)
                            {
                                node_var_t *var2 = (node_var_t *)item2->value;
                                if (syntax_objidcmp(for1->key, var2->key) == 1)
                                {
                                    syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                        item2->position.line, item2->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_IF)
                            {
                                node_if_t *if2 = (node_if_t *)item2->value;
                                if (if2->key != NULL)
                                {
                                    if (syntax_objidcmp(for1->key, if2->key) == 1)
                                    {
                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
                                        return -1;
                                    }
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_FOR)
                            {
                                node_for_t *for2 = (node_for_t *)item2->value;
                                if (for2->key != NULL)
                                {
                                    if (syntax_objidcmp(for1->key, for2->key) == 1)
                                    {
                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
                                        return -1;
                                    }
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_FORIN)
                            {
                                node_forin_t *forin2 = (node_forin_t *)item2->value;
                                if (forin2->key != NULL)
                                {
                                    if (syntax_objidcmp(for1->key, forin2->key) == 1)
                                    {
                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin1 = (node_forin_t *)item1->value;
                        if (forin1->key != NULL)
                        {
                            if (item2->kind == NODE_KIND_VAR)
                            {
                                node_var_t *var2 = (node_var_t *)item2->value;
                                if (syntax_objidcmp(forin1->key, var2->key) == 1)
                                {
                                    syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                        item2->position.line, item2->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_IF)
                            {
                                node_if_t *if2 = (node_if_t *)item2->value;
                                if (if2->key != NULL)
                                {
                                    if (syntax_objidcmp(forin1->key, if2->key) == 1)
                                    {
                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
                                        return -1;
                                    }
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_FOR)
                            {
                                node_for_t *for2 = (node_for_t *)item2->value;
                                if (for2->key != NULL)
                                {
                                    if (syntax_objidcmp(forin1->key, for2->key) == 1)
                                    {
                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
                                        return -1;
                                    }
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_FORIN)
                            {
                                node_forin_t *forin2 = (node_forin_t *)item2->value;
                                if (forin2->key != NULL)
                                {
                                    if (syntax_objidcmp(forin1->key, forin2->key) == 1)
                                    {
                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
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

    int32_t result = syntax_body(program, node_if1->then_body, frame);
    if (result == -1)
    {
        return -1;
    }

    if (node_if1->else_body != NULL)
    {
        result = syntax_body(program, node_if1->else_body, frame);
        if (result == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
syntax_for(program_t *program, node_t *node, list_t *frame)
{
    node_for_t *node_for1 = (node_for_t *)node->value;
    node_t *node1 = node_for1->body;
    node_block_t *block1 = (node_block_t *)node1->value;
    
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_VAR)
        {
            node_var_t *var1 = (node_var_t *)item1->value;
            
            node_t *node2 = node_for1->initializer;
            node_block_t *block2 = (node_block_t *)node2->value;
            
            ilist_t *b1;
            for (b1 = block2->list->begin; b1 != block2->list->end; b1 = b1->next)
            {
                node_t *item3 = (node_t *)b1->value;
                if (item3->kind == NODE_KIND_VAR)
                {
                    node_var_t *var3 = (node_var_t *)item3->value;
                    if (syntax_objidcmp(var1->key, var3->key) == 1)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item3->position.line, item3->position.column);
                        return -1;
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_IF)
        {
            node_if_t *if1 = (node_if_t *)item1->value;
            if (if1->key != NULL)
            {
                node_t *node2 = node_for1->initializer;
                node_block_t *block2 = (node_block_t *)node2->value;
                
                ilist_t *b1;
                for (b1 = block2->list->begin; b1 != block2->list->end; b1 = b1->next)
                {
                    node_t *item3 = (node_t *)b1->value;
                    if (item3->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var3 = (node_var_t *)item3->value;
                        if (syntax_objidcmp(if1->key, var3->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
                            return -1;
                        }
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_FOR)
        {
            node_for_t *for1 = (node_for_t *)item1->value;
            if (for1->key != NULL)
            {
                node_t *node2 = node_for1->initializer;
                node_block_t *block2 = (node_block_t *)node2->value;
                
                ilist_t *b1;
                for (b1 = block2->list->begin; b1 != block2->list->end; b1 = b1->next)
                {
                    node_t *item3 = (node_t *)b1->value;
                    if (item3->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var3 = (node_var_t *)item3->value;
                        if (syntax_objidcmp(for1->key, var3->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
                            return -1;
                        }
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_FORIN)
        {
            node_forin_t *forin1 = (node_forin_t *)item1->value;
            if (forin1->key != NULL)
            {
                node_t *node2 = node_for1->initializer;
                node_block_t *block2 = (node_block_t *)node2->value;
                
                ilist_t *b1;
                for (b1 = block2->list->begin; b1 != block2->list->end; b1 = b1->next)
                {
                    node_t *item3 = (node_t *)b1->value;
                    if (item3->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var3 = (node_var_t *)item3->value;
                        if (syntax_objidcmp(forin1->key, var3->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
                            return -1;
                        }
                    }
                }
            }
        }

        ilist_t *a2;
        for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;

            if (item1->id == item2->id)
            {
                break;
            }
            
            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;

                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(var1->key, var2->key) == 1)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_IF)
                {
                    node_if_t *if1 = (node_if_t *)item2->value;
                    if (if1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, if1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for1 = (node_for_t *)item2->value;
                    if (for1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, for1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FORIN)
                {
                    node_forin_t *forin1 = (node_forin_t *)item2->value;
                    if (forin1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, forin1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_IF)
            {
                node_if_t *if1 = (node_if_t *)item1->value;
                if (if1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(if1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FOR)
            {
                node_for_t *for1 = (node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(for1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FORIN)
            {
                node_forin_t *forin1 = (node_forin_t *)item1->value;
                if (forin1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(forin1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }

    int32_t result = syntax_body(program, node_for1->body, frame);
    if (result == -1)
    {
        return -1;
    }

	return 1;
}

static int32_t
syntax_forin(program_t *program, node_t *node, list_t *frame)
{
    node_forin_t *node_forin1 = (node_forin_t *)node->value;
    node_t *node1 = node_forin1->body;
    node_block_t *block1 = (node_block_t *)node1->value;
    
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_VAR)
        {
            node_var_t *var1 = (node_var_t *)item1->value;
            
            node_t *node2 = node_forin1->initializer;
            node_block_t *block2 = (node_block_t *)node2->value;
            
            ilist_t *b1;
            for (b1 = block2->list->begin; b1 != block2->list->end; b1 = b1->next)
            {
                node_t *item3 = (node_t *)b1->value;
                if (item3->kind == NODE_KIND_VAR)
                {
                    node_var_t *var3 = (node_var_t *)item3->value;
                    if (syntax_objidcmp(var1->key, var3->key) == 1)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item3->position.line, item3->position.column);
                        return -1;
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_IF)
        {
            node_if_t *if1 = (node_if_t *)item1->value;
            if (if1->key != NULL)
            {
                node_t *node2 = node_forin1->initializer;
                node_block_t *block2 = (node_block_t *)node2->value;
                
                ilist_t *b1;
                for (b1 = block2->list->begin; b1 != block2->list->end; b1 = b1->next)
                {
                    node_t *item3 = (node_t *)b1->value;
                    if (item3->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var3 = (node_var_t *)item3->value;
                        if (syntax_objidcmp(if1->key, var3->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
                            return -1;
                        }
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_FOR)
        {
            node_for_t *for1 = (node_for_t *)item1->value;
            if (for1->key != NULL)
            {
                node_t *node2 = node_forin1->initializer;
                node_block_t *block2 = (node_block_t *)node2->value;
                
                ilist_t *b1;
                for (b1 = block2->list->begin; b1 != block2->list->end; b1 = b1->next)
                {
                    node_t *item3 = (node_t *)b1->value;
                    if (item3->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var3 = (node_var_t *)item3->value;
                        if (syntax_objidcmp(for1->key, var3->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
                            return -1;
                        }
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_FORIN)
        {
            node_forin_t *forin1 = (node_forin_t *)item1->value;
            if (forin1->key != NULL)
            {
                node_t *node2 = node_forin1->initializer;
                node_block_t *block2 = (node_block_t *)node2->value;
                
                ilist_t *b1;
                for (b1 = block2->list->begin; b1 != block2->list->end; b1 = b1->next)
                {
                    node_t *item3 = (node_t *)b1->value;
                    if (item3->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var3 = (node_var_t *)item3->value;
                        if (syntax_objidcmp(forin1->key, var3->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
                            return -1;
                        }
                    }
                }
            }
        }

        ilist_t *a2;
        for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;

            if (item1->id == item2->id)
            {
                break;
            }

            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;

                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(var1->key, var2->key) == 1)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_IF)
                {
                    node_if_t *if1 = (node_if_t *)item2->value;
                    if (if1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, if1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for1 = (node_for_t *)item2->value;
                    if (for1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, for1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FORIN)
                {
                    node_forin_t *forin1 = (node_forin_t *)item2->value;
                    if (forin1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, forin1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_IF)
            {
                node_if_t *if1 = (node_if_t *)item1->value;
                if (if1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(if1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FOR)
            {
                node_for_t *for1 = (node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(for1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FORIN)
            {
                node_forin_t *forin1 = (node_forin_t *)item1->value;
                if (forin1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(forin1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }

    int32_t result = syntax_body(program, node_forin1->body, frame);
    if (result == -1)
    {
        return -1;
    }
    return 1;
}

static int32_t
syntax_parameters(program_t *program, node_t *node, list_t *frame)
{
	node_block_t *parameters = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = parameters->list->begin;a1 != parameters->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_PARAMETER)
        {
            node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
            ilist_t *a2;
            for (a2 = parameters->list->begin;a2 != parameters->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                    if (item1->id == item2->id)
                    {
                        break;
                    }
                    if (syntax_idcmp(parameter1->key, parameter2->key) == 1)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
            }

            if (parameter1->type != NULL)
            {
                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "unable to allocate memory\n");
                    return -1;
                }
                int32_t r1 = syntax_expression(program, parameter1->type, response1);
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
syntax_catch(program_t *program, node_t *node, list_t *frame)
{
    node_catch_t *node_catch1 = (node_catch_t *)node->value;
    node_t *node1 = node_catch1->body;
    node_block_t *block1 = (node_block_t *)node1->value;
    
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;

        ilist_t *a2;
        for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;
            
            if (item1->id == item2->id)
            {
                break;
            }

            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;

                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(var1->key, var2->key) == 1)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_IF)
                {
                    node_if_t *if1 = (node_if_t *)item2->value;
                    if (if1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, if1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for1 = (node_for_t *)item2->value;
                    if (for1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, for1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FORIN)
                {
                    node_forin_t *forin1 = (node_forin_t *)item2->value;
                    if (forin1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, forin1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_IF)
            {
                node_if_t *if1 = (node_if_t *)item1->value;
                if (if1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(if1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FOR)
            {
                node_for_t *for1 = (node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(for1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FORIN)
            {
                node_forin_t *forin1 = (node_forin_t *)item1->value;
                if (forin1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(forin1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }

    if (node_catch1->parameters != NULL)
    {
        int32_t r1 = syntax_parameters(program, node_catch1->parameters, frame);
        if (r1 == -1)
        {
            return -1;
        }
    }

    int32_t result = syntax_body(program, node_catch1->body, frame);
    if (result == -1)
    {
        return -1;
    }
    return 1;
}

static int32_t
syntax_try(program_t *program, node_t *node, list_t *frame)
{
    node_try_t *node_try1 = (node_try_t *)node->value;
    node_t *node1 = node_try1->body;
    node_block_t *block1 = (node_block_t *)node1->value;
    
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;

        ilist_t *a2;
        for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;
            
            if (item1->id == item2->id)
            {
                break;
            }

            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;

                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(var1->key, var2->key) == 1)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_IF)
                {
                    node_if_t *if1 = (node_if_t *)item2->value;
                    if (if1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, if1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for1 = (node_for_t *)item2->value;
                    if (for1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, for1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FORIN)
                {
                    node_forin_t *forin1 = (node_forin_t *)item2->value;
                    if (forin1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, forin1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_IF)
            {
                node_if_t *if1 = (node_if_t *)item1->value;
                if (if1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(if1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FOR)
            {
                node_for_t *for1 = (node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(for1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FORIN)
            {
                node_forin_t *forin1 = (node_forin_t *)item1->value;
                if (forin1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(forin1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }

    int32_t result = syntax_body(program, node_try1->body, frame);
    if (result == -1)
    {
        return -1;
    }

    if (node_try1->catchs != NULL)
    {
        node_t *catchs = node_try1->catchs;
        node_block_t *node_catchs = (node_block_t *)catchs->value;
        ilist_t *a2;
        for (a2 = node_catchs->list->begin;a2 != node_catchs->list->end;a2 = a2->next)
        {
            node_t *item = (node_t *)a2->value;
            int32_t result = syntax_catch(program, item, frame);
            if (result == -1)
            {
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
syntax_var(program_t *program, node_t *node, list_t *frame)
{
	return 1;
}

static int32_t
syntax_statement(program_t *program, node_t *node, list_t *frame)
{
    if (node->kind == NODE_KIND_IF)
    {
        int32_t result;
        result = syntax_if(program, node, frame);
        if (result == -1)
        {
            return -1;
        }
    }
    else 
    if (node->kind == NODE_KIND_FOR)
    {
        int32_t result;
        result = syntax_for(program, node, frame);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_FORIN)
    {
        int32_t result;
        result = syntax_forin(program, node, frame);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_TRY)
    {
        int32_t result;
        result = syntax_try(program, node, frame);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t result;
        result = syntax_var(program, node, frame);
        if (result == -1)
        {
            return -1;
        }
    }
    return 1;
}

static int32_t
syntax_body(program_t *program, node_t *node, list_t *frame)
{
    node_block_t *node_block = (node_block_t *)node->value;

    table_t *table2 = table_create();
    if (table2 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    ilist_t *fl1 = list_lpush(frame, table2);
    if (fl1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    ilist_t *a1;
    for (a1 = node_block->list->begin;a1 != node_block->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        int32_t result;
        result = syntax_statement(program, item, frame);
        if (result == -1)
        {
            return -1;
        }
    }

    ilist_t *fl2 = list_lpop(frame);
    table_t *table3 = (table_t *)fl2->value;
    table_destroy(table3);

    return 1;
}

static int32_t
syntax_generics(program_t *program, node_t *node, list_t *frame)
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
                    if (syntax_idcmp(generic1->key, generic2->key) == 1)
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
                if (response1 == NULL)
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
                if (response1 == NULL)
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
syntax_func(program_t *program, node_t *node, list_t *frame)
{
	node_func_t *node_func1 = (node_func_t *)node->value;
    
    table_t *table2 = table_create();
    if (table2 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    ilist_t *fl1 = list_lpush(frame, table2);
    if (fl1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    node_t *node1 = node_func1->body;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
    
        ilist_t *a2;
            for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;

            if (item1->id == item2->id)
            {
                break;
            }

            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(var1->key, var2->key) == 1)
                    {
                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_IF)
                {
                    node_if_t *if1 = (node_if_t *)item2->value;
                    if (if1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, if1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for1 = (node_for_t *)item2->value;
                    if (for1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, for1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FORIN)
                {
                    node_forin_t *forin1 = (node_forin_t *)item2->value;
                    if (forin1->key != NULL)
                    {
                        if (syntax_objidcmp(var1->key, forin1->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_IF)
            {
                node_if_t *if1 = (node_if_t *)item1->value;
                if (if1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(if1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(if1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FOR)
            {
                node_for_t *for1 = (node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(for1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(for1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FORIN)
            {
                node_forin_t *forin1 = (node_forin_t *)item1->value;
                if (forin1->key != NULL)
                {
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (syntax_objidcmp(forin1->key, var2->key) == 1)
                        {
                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                item2->position.line, item2->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, if2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, for2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *forin2 = (node_forin_t *)item2->value;
                        if (forin2->key != NULL)
                        {
                            if (syntax_objidcmp(forin1->key, forin2->key) == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }



    if (node_func1->generics != NULL)
    {
        int32_t r1 = syntax_generics(program, node_func1->generics, frame);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (node_func1->parameters != NULL)
    {
        int32_t r1 = syntax_parameters(program, node_func1->parameters, frame);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (node_func1->body != NULL)
    {
        int32_t r1 = syntax_body(program, node_func1->body, frame);
        if (r1 == -1)
        {
            return -1;
        }
    }

    ilist_t *fl2 = list_lpop(frame);
    table_t *table3 = (table_t *)fl2->value;
    table_destroy(table3);

	return 1;
}

static int32_t
syntax_enum(program_t *program, node_t *node, list_t *frame)
{
	node_enum_t *node_enum1 = (node_enum_t *)node->value;
    node_t *node1 = node_enum1->members;
    node_block_t *block1 = (node_block_t *)node1->value;
    
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        ilist_t *a2;
        for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;
            if (item1->id == item2->id)
            {
                break;
            }
            
            node_member_t *member1 = (node_member_t *)item1->value;
            node_member_t *member2 = (node_member_t *)item2->value;
            if (syntax_idcmp(member1->key, member2->key) == 1)
            {
                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                    item2->position.line, item2->position.column);
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
syntax_import(program_t *program, node_t *node, list_t *frame)
{
	//node_import_t *node_import = (node_import_t *)node->value;
	return 1;
}

static int32_t
syntax_property(program_t *program, node_t *node, list_t *frame)
{
	node_property_t *node_property1 = (node_property_t *)node->value;
    node_t *parent = node->parent;
    region_start:
    if (parent != NULL)
    {
        if (parent->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)parent->value;
            node_t *node2 = class1->block;
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
                    if (syntax_idcmp(node_property1->key, node_class2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = node_class2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = node_class2->block;
                            node_block_t *block2 = (node_block_t *)node3->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item2->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = fun1->parameters;
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
                if (item->kind == NODE_KIND_FUNC)
                {
                    node_func_t *node_func2 = (node_func_t *)item->value;
                    if (syntax_idcmp(node_property1->key, node_func2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = node_func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = node_func2->parameters;
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
                else
                if (item->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *node_enum2 = (node_enum_t *)item->value;
                    if (syntax_idcmp(node_property1->key, node_enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
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
                else
                if (item->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *node_property2 = (node_property_t *)item->value;
                    if (syntax_idcmp(node_property1->key, node_property2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
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

            if (class1->generics != NULL)
            {
                node_t *node3 = (node_t *)class1->generics;
                node_block_t *generics = (node_block_t *)node3->value;
                ilist_t *a1;
                for (a1 = generics->list->begin;a1 != generics->list->end;a1 = a1->next)
                {
                    node_t *item1 = (node_t *)a1->value;
                    if (item1->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic1 = (node_generic_t *)item1->value;
                        if (syntax_idcmp(node_property1->key, generic1->key) == 1)
                        {
                            node_t *ngs1 = NULL;
                            node_t *ngs2 = NULL;
                            int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                node_t *nps1 = NULL;
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
                                        item1->position.line, item1->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
            }

            if (class1->heritages != NULL)
            {
                node_t *node3 = class1->heritages;
                node_block_t *heritages = (node_block_t *)node3->value;
                ilist_t *a1;
                for (a1 = heritages->list->begin;a1 != heritages->list->end;a1 = a1->next)
                {
                    node_t *item1 = (node_t *)a1->value;
                    if (item1->kind == NODE_KIND_HERITAGE)
                    {
                        node_heritage_t *heritage1 = (node_heritage_t *)item1->value;
                        if (syntax_idcmp(node_property1->key, heritage1->key) == 1)
                        {
                            node_t *ngs1 = NULL;
                            node_t *ngs2 = NULL;
                            int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                node_t *nps1 = NULL;
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
                                        item1->position.line, item1->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
            }

        }
    }
	
    if (parent->parent)
    {
        if ((parent->kind == NODE_KIND_CLASS) || (parent->kind == NODE_KIND_MODULE))
        {
            goto region_end;
        }
        parent = parent->parent;
        goto region_start;
    }
    region_end:

    if (node_property1->type != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "unable to allocate memory\n");
            return -1;
        }
        int32_t r1 = syntax_expression(program, node_property1->type, response1);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            syntax_error(program, node, "reference not found");
            return -1;
        }
        else
        {
            if (list_count(response1) > 1)
            {
                syntax_error(program, node, "multiple reference");
                return -1;
            }
            list_destroy(response1);
        }
    }

	return 1;
}

static int32_t
syntax_heritages(program_t *program, node_t *node, list_t *frame)
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
                    if (syntax_idcmp(heritage1->key, heritage2->key) == 1)
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
                if (response1 == NULL)
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
syntax_class(program_t *program, node_t *node, list_t *frame)
{
	node_class_t *node_class = (node_class_t *)node->value;


    table_t *table2 = table_create();
    if (table2 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    ilist_t *fl1 = list_lpush(frame, table2);
    if (fl1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }


    if (node_class->generics != NULL)
    {
        int32_t r1 = syntax_generics(program, node_class->generics, frame);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (node_class->heritages != NULL)
    {
        int32_t r1 = syntax_heritages(program, node_class->heritages, frame);
        if (r1 == -1)
        {
            return -1;
        }
    }

    node_t *node1 = node_class->block;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)item1->value;

            ilist_t *a2;
            for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item1->id == item2->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class2 = (node_class_t *)item2->value;

                    node_t *node3 = (node_t *)class1->generics;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *b1;
                    for (b1 = block3->list->begin;b1 != block3->list->end;b1 = b1->next)
                    {
                        node_t *ng1 = (node_t *)b1->value;
                        if (ng1->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)ng1->value;
                            if (syntax_idcmp(class2->key, generic1->key) == 1)
                            {
                                node_t *ngs1 = class1->generics;
                                node_t *ngs2 = NULL;
                                int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    node_t *node2 = class2->block;
                                    node_block_t *block2 = (node_block_t *)node2->value;
                                    ilist_t *b1;
                                    for (b1 = block2->list->begin;b1 != block2->list->end;b1 = b1->next)
                                    {
                                        node_t *item3 = (node_t *)b1->value;
                                        if (item3->kind == NODE_KIND_FUNC)
                                        {
                                            node_func_t *fun1 = (node_func_t *)item3->value;
                                            if (syntax_idstrcmp(fun1->key, "constructor") == 1)
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
                                                    syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                                        ng1->position.line, ng1->position.column);
                                                    return -1;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    node_t *node4 = (node_t *)class1->heritages;
                    node_block_t *block4 = (node_block_t *)node4->value;

                    ilist_t *b2;
                    for (b2 = block4->list->begin;b2 != block4->list->end;b2 = b2->next)
                    {
                        node_t *nh1 = (node_t *)b2->value;
                        if (nh1->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage1 = (node_heritage_t *)nh1->value;
                            if (syntax_idcmp(class2->key, heritage1->key) == 1)
                            {
                                node_t *ngs1 = class1->generics;
                                node_t *ngs2 = NULL;
                                int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    node_t *node2 = class2->block;
                                    node_block_t *block2 = (node_block_t *)node2->value;
                                    ilist_t *b1;
                                    for (b1 = block2->list->begin;b1 != block2->list->end;b1 = b1->next)
                                    {
                                        node_t *item3 = (node_t *)b1->value;
                                        if (item3->kind == NODE_KIND_FUNC)
                                        {
                                            node_func_t *fun1 = (node_func_t *)item3->value;
                                            if (syntax_idstrcmp(fun1->key, "constructor") == 1)
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
                                                    syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                                        nh1->position.line, nh1->position.column);
                                                    return -1;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (syntax_idcmp(class1->key, class2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = class2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *b1;
                            for (b1 = block2->list->begin;b1 != block2->list->end;b1 = b1->next)
                            {
                                node_t *item3 = (node_t *)b1->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 1)
                                    {
                                        node_t *node3 = class2->block;
                                        node_block_t *block3 = (node_block_t *)node3->value;
                                        ilist_t *b2;
                                        for (b2 = block3->list->begin;b2 != block3->list->end;b2 = b2->next)
                                        {
                                            node_t *item4 = (node_t *)b2->value;
                                            if (item4->kind == NODE_KIND_FUNC)
                                            {
                                                node_func_t *fun2 = (node_func_t *)item4->value;
                                                if (syntax_idstrcmp(fun2->key, "constructor") == 1)
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
                                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                            item2->position.line, item2->position.column);
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
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum1 = (node_enum_t *)item2->value;

                    node_t *node3 = (node_t *)class1->generics;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *b1;
                    for (b1 = block3->list->begin;b1 != block3->list->end;b1 = b1->next)
                    {
                        node_t *ng1 = (node_t *)b1->value;
                        if (ng1->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)ng1->value;
                            if (syntax_idcmp(enum1->key, generic1->key) == 1)
                            {
                                node_t *ngs1 = NULL;
                                node_t *ngs2 = NULL;
                                int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    node_t *nps1 = NULL;
                                    node_t *nps2 = NULL;
                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                            ng1->position.line, ng1->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }

                    node_t *node4 = (node_t *)class1->heritages;
                    node_block_t *block4 = (node_block_t *)node4->value;

                    ilist_t *b2;
                    for (b2 = block4->list->begin;b2 != block4->list->end;b2 = b2->next)
                    {
                        node_t *nh1 = (node_t *)b2->value;
                        if (nh1->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage1 = (node_heritage_t *)nh1->value;
                            if (syntax_idcmp(enum1->key, heritage1->key) == 1)
                            {
                                node_t *ngs1 = NULL;
                                node_t *ngs2 = NULL;
                                int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    node_t *nps1 = NULL;
                                    node_t *nps2 = NULL;
                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                            nh1->position.line, nh1->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }

                    if (syntax_idcmp(class1->key, enum1->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *b1;
                            for (b1 = block2->list->begin;b1 != block2->list->end;b1 = b1->next)
                            {
                                node_t *item3 = (node_t *)b1->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 1)
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
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func1 = (node_func_t *)item2->value;

                    node_t *node3 = (node_t *)class1->generics;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *b1;
                    for (b1 = block3->list->begin;b1 != block3->list->end;b1 = b1->next)
                    {
                        node_t *ng1 = (node_t *)b1->value;
                        if (ng1->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)ng1->value;
                            if (syntax_idcmp(func1->key, generic1->key) == 1)
                            {
                                node_t *ngs1 = func1->generics;
                                node_t *ngs2 = NULL;
                                int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    node_t *nps1 = func1->parameters;
                                    node_t *nps2 = NULL;
                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                            ng1->position.line, ng1->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }

                    node_t *node4 = (node_t *)class1->heritages;
                    node_block_t *block4 = (node_block_t *)node4->value;

                    ilist_t *b2;
                    for (b2 = block4->list->begin;b2 != block4->list->end;b2 = b2->next)
                    {
                        node_t *nh1 = (node_t *)b2->value;
                        if (nh1->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage1 = (node_heritage_t *)nh1->value;
                            if (syntax_idcmp(func1->key, heritage1->key) == 1)
                            {
                                node_t *ngs1 = NULL;
                                node_t *ngs2 = NULL;
                                int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    node_t *nps1 = NULL;
                                    node_t *nps2 = NULL;
                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                            nh1->position.line, nh1->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }

                    if (syntax_idcmp(class1->key, func1->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = func1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func2 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func2->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func2->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property1 = (node_property_t *)item2->value;

                    node_t *node3 = (node_t *)class1->generics;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *b1;
                    for (b1 = block3->list->begin;b1 != block3->list->end;b1 = b1->next)
                    {
                        node_t *ng1 = (node_t *)b1->value;
                        if (ng1->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)ng1->value;
                            if (syntax_idcmp(property1->key, generic1->key) == 1)
                            {
                                node_t *ngs1 = NULL;
                                node_t *ngs2 = NULL;
                                int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    node_t *nps1 = NULL;
                                    node_t *nps2 = NULL;
                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                            ng1->position.line, ng1->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }

                    node_t *node4 = (node_t *)class1->heritages;
                    node_block_t *block4 = (node_block_t *)node4->value;

                    ilist_t *b2;
                    for (b2 = block4->list->begin;b2 != block4->list->end;b2 = b2->next)
                    {
                        node_t *nh1 = (node_t *)b2->value;
                        if (nh1->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage1 = (node_heritage_t *)nh1->value;
                            if (syntax_idcmp(property1->key, heritage1->key) == 1)
                            {
                                node_t *ngs1 = NULL;
                                node_t *ngs2 = NULL;
                                int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    node_t *nps1 = NULL;
                                    node_t *nps2 = NULL;
                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                            nh1->position.line, nh1->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }

                    if (syntax_idcmp(class1->key, property1->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *b1;
                            for (b1 = block2->list->begin;b1 != block2->list->end;b1 = b1->next)
                            {
                                node_t *item3 = (node_t *)b1->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 1)
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
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
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
        else
        if (item1->kind == NODE_KIND_FUNC)
        {
            node_func_t *func1 = (node_func_t *)item1->value;

            ilist_t *a2;
            for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item1->id == item2->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class2 = (node_class_t *)item2->value;
                    if (syntax_idcmp(func1->key, class2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = class2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class2->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func2 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func2->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func2->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func2 = (node_func_t *)item2->value;
                    if (syntax_idcmp(func1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = func2->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;
                    if (syntax_idcmp(func1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property1 = (node_property_t *)item2->value;
                    if (syntax_idcmp(func1->key, property1->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_ENUM)
        {
            ilist_t *a2;
            for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item1->id == item2->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    node_class_t *class1 = (node_class_t *)item2->value;
                    if (syntax_idcmp(enum1->key, class1->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func1->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func1->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    node_func_t *func1 = (node_func_t *)item2->value;
                    if (syntax_idcmp(enum1->key, func1->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = func1->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    node_enum_t *enum2 = (node_enum_t *)item2->value;
                    if (syntax_idcmp(enum1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    node_property_t *property1 = (node_property_t *)item2->value;
                    if (syntax_idcmp(enum1->key, property1->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_PROPERTY)
        {
            ilist_t *a2;
            for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item1->id == item2->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_property_t *property1 = (node_property_t *)item1->value;
                    node_class_t *class1 = (node_class_t *)item2->value;
                    if (syntax_idcmp(property1->key, class1->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func1->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func1->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_property_t *property1 = (node_property_t *)item1->value;
                    node_func_t *func1 = (node_func_t *)item2->value;
                    if (syntax_idcmp(property1->key, func1->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = func1->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_property_t *property1 = (node_property_t *)item1->value;
                    node_enum_t *enum2 = (node_enum_t *)item2->value;
                    if (syntax_idcmp(property1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property1 = (node_property_t *)item1->value;
                    node_property_t *property2 = (node_property_t *)item2->value;
                    if (syntax_idcmp(property1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }

    int32_t constructor_defined = 0;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        if (item->kind == NODE_KIND_CLASS)
        {
            int32_t result;
            result = syntax_class(program, item, frame);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_ENUM)
        {
            int32_t result;
            result = syntax_enum(program, item, frame);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_FUNC)
        {
            int32_t result;
            result = syntax_func(program, item, frame);
            if (result == -1)
            {
                return -1;
            }

            node_func_t *node_func = (node_func_t *)item->value;
            if (syntax_idstrcmp(node_func->key, "constructor") == 1)
            {
                constructor_defined = 1;
            }
        }
        else
        if (item->kind == NODE_KIND_PROPERTY)
        {
            int32_t result;
            result = syntax_property(program, item, frame);
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

    ilist_t *fl2 = list_lpop(frame);
    table_t *table3 = (table_t *)fl2->value;
    table_destroy(table3);

	return 1;
}

static int32_t
syntax_module(program_t *program, node_t *node, list_t *frame)
{
	node_module_t *module = (node_module_t *)node->value;
    
    node_t *node1 = module->block;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin; a1 != block1->list->end; a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_CLASS)
        {
            ilist_t *a2;
            for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item1->id == item2->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)item1->value;
                    node_class_t *class2 = (node_class_t *)item2->value;
                    if (syntax_idcmp(class1->key, class2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = class2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *b1;
                            for (b1 = block2->list->begin;b1 != block2->list->end;b1 = b1->next)
                            {
                                node_t *item3 = (node_t *)b1->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 1)
                                    {
                                        node_t *node3 = class2->block;
                                        node_block_t *block3 = (node_block_t *)node3->value;
                                        ilist_t *b2;
                                        for (b2 = block3->list->begin;b2 != block3->list->end;b2 = b2->next)
                                        {
                                            node_t *item4 = (node_t *)b2->value;
                                            if (item4->kind == NODE_KIND_FUNC)
                                            {
                                                node_func_t *fun2 = (node_func_t *)item4->value;
                                                if (syntax_idstrcmp(fun2->key, "constructor") == 1)
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
                                                        syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                            item2->position.line, item2->position.column);
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
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_class_t *class1 = (node_class_t *)item1->value;
                    node_enum_t *enum1 = (node_enum_t *)item2->value;
                    if (syntax_idcmp(class1->key, enum1->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *b1;
                            for (b1 = block2->list->begin;b1 != block2->list->end;b1 = b1->next)
                            {
                                node_t *item3 = (node_t *)b1->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *fun1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(fun1->key, "constructor") == 1)
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
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_class_t *class1 = (node_class_t *)item1->value;
                    node_func_t *func1 = (node_func_t *)item2->value;
                    if (syntax_idcmp(class1->key, func1->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = func1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func2 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func2->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func2->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_class_t *class1 = (node_class_t *)item1->value;
                    node_var_t *var1 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(class1->key, var1->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func1->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = NULL;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
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
        else
        if (item1->kind == NODE_KIND_FUNC)
        {
            ilist_t *a2;
            for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item1->id == item2->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_func_t *func1 = (node_func_t *)item1->value;
                    node_class_t *class2 = (node_class_t *)item2->value;
                    if (syntax_idcmp(func1->key, class2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = class2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class2->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func2 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func2->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func2->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func1 = (node_func_t *)item1->value;
                    node_func_t *func2 = (node_func_t *)item2->value;
                    if (syntax_idcmp(func1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = func2->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_func_t *func1 = (node_func_t *)item1->value;
                    node_enum_t *enum2 = (node_enum_t *)item2->value;
                    if (syntax_idcmp(func1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_func_t *func1 = (node_func_t *)item1->value;
                    node_var_t *var1 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(func1->key, var1->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_ENUM)
        {
            ilist_t *a2;
            for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item1->id == item2->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    node_class_t *class1 = (node_class_t *)item2->value;
                    if (syntax_idcmp(enum1->key, class1->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func1->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func1->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    node_func_t *func1 = (node_func_t *)item2->value;
                    if (syntax_idcmp(enum1->key, func1->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = func1->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    node_enum_t *enum2 = (node_enum_t *)item2->value;
                    if (syntax_idcmp(enum1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    node_var_t *var1 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(enum1->key, var1->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
        else
        if (item1->kind == NODE_KIND_VAR)
        {
            ilist_t *a2;
            for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item1->id == item2->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_var_t *var1 = (node_var_t *)item1->value;
                    node_class_t *class1 = (node_class_t *)item2->value;
                    if (syntax_objidcmp(var1->key, class1->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class1->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = class1->block;
                            node_block_t *block2 = (node_block_t *)node2->value;
                            ilist_t *a2;
                            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func1->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func1->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_var_t *var1 = (node_var_t *)item1->value;
                    node_func_t *func2 = (node_func_t *)item2->value;
                    if (syntax_objidcmp(var1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = func2->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_var_t *var1 = (node_var_t *)item1->value;
                    node_enum_t *enum2 = (node_enum_t *)item2->value;
                    if (syntax_objidcmp(var1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var1 = (node_var_t *)item1->value;
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (syntax_objidcmp(var1->key, var2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }

    for (a1 = block1->list->begin; a1 != block1->list->end; a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        if (item->kind == NODE_KIND_IMPORT)
        {
            int32_t result;
            result = syntax_import(program, item, frame);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_CLASS)
        {
            int32_t result;
            result = syntax_class(program, item, frame);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_ENUM)
        {
            int32_t result;
            result = syntax_enum(program, item, frame);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_FUNC)
        {
            int32_t result;
            result = syntax_func(program, item, frame);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_VAR)
        {
            int32_t result;
            result = syntax_var(program, item, frame);
            if (result == -1)
            {
                return -1;
            }
        }
    }

	return 1;
}

int32_t
syntax_run(program_t *program, node_t *node)
{
    list_t *frame = list_create();
    if (frame == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    table_t *table = table_create();
    if (table == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

	int32_t result = syntax_module(program, node, frame);
	if(result == -1)
	{
		return -1;
	}
	return 1;
}
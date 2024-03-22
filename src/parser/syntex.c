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
        node_block_t *node_object = (node_block_t *)n1->value;
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
syntax_postfix(program_t *program, node_t *node, list_t *response);


static int32_t
syntax_body(program_t *program, node_t *node);



static int32_t
syntax_eqaul_gsgs(program_t *program, node_t *ngs1, node_t *ngs2)
{
    return 1;
}

static int32_t
syntax_eqaul_gscs(program_t *program, node_t *ngs1, node_t *nas2)
{
    return 1;
}

static int32_t
syntax_eqaul_psps(program_t *program, node_t *nps1, node_t *nps2)
{
    return 1;
}

static int32_t
syntax_eqaul_psas(program_t *program, node_t *nps1, node_t *nds2)
{
    return 1;
}




static int32_t
syntax_number(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_char(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_string(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_true(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_false(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_infinity(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_array(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_object(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_this(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_null(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_id(program_t *program, node_t *node, list_t *response)
{
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_BODY)
        {
            node_block_t *block1 = (node_block_t *)current->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_IF)
                {
                    node_if_t *if1 = (node_if_t *)item1->value;
                    if (if1->key != NULL)
                    {
                        if (syntax_idcmp(node, if1->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item1);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
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
                        if (syntax_idcmp(node, for1->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item1);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
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
                        if (syntax_idcmp(node, forin1->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item1);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_VAR)
                {
                    node_var_t *var1 = (node_var_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (syntax_idcmp(node, var1->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item1);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        node_t *node2 = (node_t *)var1->key;
                        node_block_t *block2 = (node_block_t *)node2->value;
                        ilist_t *a2;
                        for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                        {
                            node_t *item2 = (node_t *)a2->value;
                            if (item2->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property2 = (node_property_t *)item2->value;
                                if (property2->type == NULL)
                                {
                                    if (syntax_idcmp(node, property2->key) == 1)
                                    {
                                        ilist_t *r1 = list_rpush(response, item2);
                                        if (r1 == NULL)
                                        {
                                            fprintf(stderr, "unable to allocate memory\n");
                                            return -1;
                                        }
                                    }
                                }
                                else
                                {
                                    if (syntax_idcmp(node, property2->type) == 1)
                                    {
                                        ilist_t *r1 = list_rpush(response, item2);
                                        if (r1 == NULL)
                                        {
                                            fprintf(stderr, "unable to allocate memory\n");
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
        if (current->kind == NODE_KIND_FUNC)
        {
            node_func_t *func2 = (node_func_t *)current->value;
            if (func2->generics != NULL)
            {
                node_t *node2 = func2->generics;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item2->value;
                        if (syntax_idcmp(node, generic3->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item2);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                        }
                    }
                }
            }

            if (func2->parameters != NULL)
            {
                node_t *node3 = func2->parameters;
                node_block_t *block3 = (node_block_t *)node3->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                        if (syntax_idcmp(node, parameter3->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item3);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                        }
                    }
                }
            }
        }
        else
        if (current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)current->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;
            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1p = (node_class_t *)item1->value;
                    if (syntax_idcmp(node, class1p->key) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    if (syntax_idcmp(node, enum1->key) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func1 = (node_func_t *)item1->value;
                    if (syntax_idcmp(node, func1->key) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property1 = (node_property_t *)item1->value;
                    if (syntax_idcmp(node, property1->key) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                    }
                }
            }

            if (class1->generics != NULL)
            {
                node_t *node2 = class1->generics;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item2->value;
                        if (syntax_idcmp(node, generic3->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item2);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                        }
                    }
                }
            }

            if (class1->heritages != NULL)
            {
                node_t *node2 = class1->heritages;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_HERITAGE)
                    {
                        node_heritage_t *heritage3 = (node_heritage_t *)item2->value;
                        if (syntax_idcmp(node, heritage3->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item2);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                        }
                    }
                }
            }

            if (class1->heritages != NULL)
            {
                node_t *node2 = class1->heritages;
                node_block_t *block2 = (node_block_t *)node2->value;

                list_t *repository = list_create();
                if (repository == NULL)
                {
                    fprintf(stderr, "unable to allocate memory\n");
                    return -1;
                }
                list_t *repository_list = list_create();
                if (repository_list == NULL)
                {
                    fprintf(stderr, "unable to allocate memory\n");
                    return -1;
                }

                int32_t start_new = 0;

                ilist_t *a2 = block2->list->begin;
                while (a2 != NULL)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_HERITAGE)
                    {
                        node_heritage_t *heritage3 = (node_heritage_t *)item2->value;
                        
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                        int32_t r1 = syntax_postfix(program, heritage3->type, response2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            syntax_error(program, heritage3->key, "reference not found");
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *a3;
                            for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_CLASS)
                                {
                                    node_class_t *class4 = (node_class_t *)item3->value;
                                    node_t *node4 = class4->block;
                                    node_block_t *block4 = (node_block_t *)node4->value;
                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_CLASS)
                                        {
                                            node_class_t *class1p = (node_class_t *)item4->value;
                                            if (syntax_idcmp(node, class1p->key) == 1)
                                            {
                                                ilist_t *r1 = list_rpush(response, item4);
                                                if (r1 == NULL)
                                                {
                                                    fprintf(stderr, "unable to allocate memory\n");
                                                    return -1;
                                                }
                                            }
                                        }
                                        else
                                        if (item4->kind == NODE_KIND_ENUM)
                                        {
                                            node_enum_t *enum1 = (node_enum_t *)item4->value;
                                            if (syntax_idcmp(node, enum1->key) == 1)
                                            {
                                                ilist_t *r1 = list_rpush(response, item4);
                                                if (r1 == NULL)
                                                {
                                                    fprintf(stderr, "unable to allocate memory\n");
                                                    return -1;
                                                }
                                            }
                                        }
                                        else
                                        if (item4->kind == NODE_KIND_FUNC)
                                        {
                                            node_func_t *func1 = (node_func_t *)item4->value;
                                            if (syntax_idcmp(node, func1->key) == 1)
                                            {
                                                ilist_t *r1 = list_rpush(response, item4);
                                                if (r1 == NULL)
                                                {
                                                    fprintf(stderr, "unable to allocate memory\n");
                                                    return -1;
                                                }
                                            }
                                        }
                                        else
                                        if (item4->kind == NODE_KIND_PROPERTY)
                                        {
                                            node_property_t *property1 = (node_property_t *)item4->value;
                                            if (syntax_idcmp(node, property1->key) == 1)
                                            {
                                                ilist_t *r1 = list_rpush(response, item4);
                                                if (r1 == NULL)
                                                {
                                                    fprintf(stderr, "unable to allocate memory\n");
                                                    return -1;
                                                }
                                            }
                                        }
                                    }

                                    ilist_t *r5 = list_rpush(repository_list, a2);
                                    if (r5 == NULL)
                                    {
                                        fprintf(stderr, "unable to allocate memory\n");
                                        return -1;
                                    }

                                    ilist_t *r6 = list_rpush(repository, node2);
                                    if (r6 == NULL)
                                    {
                                        fprintf(stderr, "unable to allocate memory\n");
                                        return -1;
                                    }

                                    node2 = class4->heritages;
                                    block2 = (node_block_t *)node2->value;
                                    a2 = block2->list->begin;
                                    start_new = 1;
                                    break;
                                }
                            }
                        }
                    
                    }

                    if (a2 == block2->list->end)
                    {
                        ilist_t *r5 = list_rpop(repository_list);
                        if (r5 != NULL)
                        {
                            a2 = (ilist_t *)r5->value;
                        }
                        ilist_t *r6 = list_rpop(repository);
                        if (r6 != NULL)
                        {
                            node2 = (node_t *)r6->value;
                            block2 = (node_block_t *)node2->value;
                            continue;
                        }
                        break;
                    }
                    else
                    {
                        if (start_new != 1)
                        {
                            a2 = a2->next;
                        }
                        start_new = 0;
                    }
                }

                list_destroy(repository_list);
                list_destroy(repository);
            }
        }
        else
        if (current->kind == NODE_KIND_MODULE)
        {
            node_class_t *class1 = (node_class_t *)current->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;
            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1p = (node_class_t *)item1->value;
                    if (syntax_idcmp(node, class1p->key) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum1 = (node_enum_t *)item1->value;
                    if (syntax_idcmp(node, enum1->key) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                    }
                }
            }
        }
        current = current->parent;
    }
}

static int32_t
syntax_primary(program_t *program, node_t *node, list_t *response)
{
    if (node->kind == NODE_KIND_ID)
    {
        return syntax_id(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_NUMBER)
    {
        return syntax_number(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_CHAR)
    {
        return syntax_char(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_STRING)
    {
        return syntax_string(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_NULL)
    {
        return syntax_null(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_TRUE)
    {
        return syntax_true(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_FALSE)
    {
        return syntax_false(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_INFINITY)
    {
        return syntax_infinity(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_THIS)
    {
        return syntax_this(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_ARRAY)
    {
        return syntax_array(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_OBJECT)
    {
        return syntax_object(program, node, response);
    }
    return 1;
}

static int32_t
syntax_pseudonym(program_t *program, node_t *node, list_t *response)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = syntax_postfix(program, carrier->base, response1);
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
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)item1->value;
                node_t *ngs1 = class1->generics;
                node_t *nas2 = carrier->data;
                int32_t r1 = syntax_eqaul_gscs(program, ngs1, nas2);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 1)
                {
                    node_t *clone1 = node_clone(item1);
                    if (clone1 == NULL)
                    {
                        fprintf(stderr, "unable to allocate memory\n");
                        return -1;
                    }
                    clone1->concepts = carrier->data;
                    ilist_t *il1 = list_rpush(response, clone1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "unable to allocate memory\n");
                        return -1;
                    }
                    return 1;
                }
            }
            else
            if (item1->kind == NODE_KIND_FUNC)
            {
                node_func_t *func1 = (node_func_t *)item1;
                node_t *ngs1 = func1->generics;
                node_t *nas2 = carrier->data;
                int32_t r1 = syntax_eqaul_gscs(program, ngs1, nas2);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 1)
                {
                    node_t *clone1 = node_clone(item1);
                    if (clone1 == NULL)
                    {
                        fprintf(stderr, "unable to allocate memory\n");
                        return -1;
                    }
                    clone1->concepts = carrier->data;
                    ilist_t *il1 = list_rpush(response, clone1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "unable to allocate memory\n");
                        return -1;
                    }
                    return 1;
                }
            }
            else
            {
                syntax_error(program, node, "generic type used for (%lld:%lld)",
                    item1->position.line, item1->position.column);
                return -1;
            }
        }
    }

    list_destroy(response1);
    return 0;
}

static int32_t
syntax_call(program_t *program, node_t *node, list_t *response)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = syntax_postfix(program, carrier->base, response1);
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
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)item1->value;
                node_t *node1 = class1->block;
                node_block_t *block1 = (node_block_t *)node1->value;
                ilist_t *b1;
                for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                {
                    node_t *item2 = (node_t *)b1->value;
                    if (item2->kind == NODE_KIND_FUNC)
                    {
                        node_func_t *func1 = (node_func_t *)item2->value;
                        if (syntax_idstrcmp(func1->key, "constructor") == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nds2 = carrier->data;
                            int32_t r2 = syntax_eqaul_psas(program, nps1, nds2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                node_t *clone1 = node_clone(item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "unable to allocate memory\n");
                                    return -1;
                                }
                                clone1->arguments = carrier->data;
                                ilist_t *il1 = list_rpush(response, clone1);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "unable to allocate memory\n");
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FUNC)
            {
                node_func_t *func1 = (node_func_t *)item1->value;
                node_t *nps1 = func1->parameters;
                node_t *nds2 = carrier->data;
                int32_t r1 = syntax_eqaul_psas(program, nps1, nds2);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 1)
                {
                    ilist_t *il1 = list_rpush(response, item1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "unable to allocate memory\n");
                        return -1;
                    }
                    list_destroy(response1);
                    return 1;
                }
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                if (item1->referto == NULL)
                {
                    syntax_error(program, item1, "not initialized");
                    return -1;
                }
                node_t *referto = item1->referto;
                if (referto->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)referto->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *b1;
                    for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                    {
                        node_t *item2 = (node_t *)b1->value;
                        if (item2->kind == NODE_KIND_FUNC)
                        {
                            node_func_t *func1 = (node_func_t *)item2->value;
                            if (syntax_idstrcmp(func1->key, "constructor") == 1)
                            {
                                node_t *nps1 = func1->parameters;
                                node_t *nds2 = carrier->data;
                                int32_t r2 = syntax_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    ilist_t *il1 = list_rpush(response, referto);
                                    if (il1 == NULL)
                                    {
                                        fprintf(stderr, "unable to allocate memory\n");
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                            }
                        }
                    }
                }
                else
                if (referto->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func1 = (node_func_t *)referto->value;
                    node_t *nps1 = func1->parameters;
                    node_t *nds2 = carrier->data;
                    int32_t r1 = syntax_eqaul_psas(program, nps1, nds2);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r1 == 1)
                    {
                        ilist_t *il1 = list_rpush(response, referto);
                        if (il1 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                        list_destroy(response1);
                        return 1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                if (item1->referto == NULL)
                {
                    syntax_error(program, item1, "not initialized");
                    return -1;
                }
                node_t *referto = item1->referto;
                if (referto->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)referto->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *b1;
                    for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                    {
                        node_t *item2 = (node_t *)b1->value;
                        if (item2->kind == NODE_KIND_FUNC)
                        {
                            node_func_t *func1 = (node_func_t *)item2->value;
                            if (syntax_idstrcmp(func1->key, "constructor") == 1)
                            {
                                node_t *nps1 = func1->parameters;
                                node_t *nds2 = carrier->data;
                                int32_t r2 = syntax_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    ilist_t *il1 = list_rpush(response, referto);
                                    if (il1 == NULL)
                                    {
                                        fprintf(stderr, "unable to allocate memory\n");
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                            }
                        }
                    }
                }
                else
                if (referto->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func1 = (node_func_t *)referto->value;
                    node_t *nps1 = func1->parameters;
                    node_t *nds2 = carrier->data;
                    int32_t r1 = syntax_eqaul_psas(program, nps1, nds2);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r1 == 1)
                    {
                        ilist_t *il1 = list_rpush(response, referto);
                        if (il1 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                        list_destroy(response1);
                        return 1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                if (item1->referto == NULL)
                {
                    syntax_error(program, item1, "not initialized");
                    return -1;
                }
                node_t *referto = item1->referto;
                if (referto->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)referto->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *b1;
                    for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                    {
                        node_t *item2 = (node_t *)b1->value;
                        if (item2->kind == NODE_KIND_FUNC)
                        {
                            node_func_t *func1 = (node_func_t *)item2->value;
                            if (syntax_idstrcmp(func1->key, "constructor") == 1)
                            {
                                node_t *nps1 = func1->parameters;
                                node_t *nds2 = carrier->data;
                                int32_t r2 = syntax_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    ilist_t *il1 = list_rpush(response, referto);
                                    if (il1 == NULL)
                                    {
                                        fprintf(stderr, "unable to allocate memory\n");
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                            }
                        }
                    }
                }
                else
                if (referto->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func1 = (node_func_t *)referto->value;
                    node_t *nps1 = func1->parameters;
                    node_t *nds2 = carrier->data;
                    int32_t r1 = syntax_eqaul_psas(program, nps1, nds2);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r1 == 1)
                    {
                        ilist_t *il1 = list_rpush(response, referto);
                        if (il1 == NULL)
                        {
                            fprintf(stderr, "unable to allocate memory\n");
                            return -1;
                        }
                        list_destroy(response1);
                        return 1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_HERITAGE)
            {
                node_heritage_t *heritage1 = (node_heritage_t *)item1->value;
                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "unable to allocate memory\n");
                    return -1;
                }
                int32_t r2 = syntax_postfix(program, heritage1->type, response2);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 0)
                {
                    syntax_error(program, heritage1->type, "reference not found");
                    return -1;
                }
                else
                if (r2 == 1)
                {
                    ilist_t *a2;
                    for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class2 = (node_class_t *)item2->value;
                            node_t *node2 = class2->block;
                            node_block_t *block2 = (node_block_t *)node2->value;

                            ilist_t *a3;
                            for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func1->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nds2 = carrier->data;
                                        int32_t r2 = syntax_eqaul_psas(program, nps1, nds2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            node_t *clone1 = node_clone(item2);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "unable to allocate memory\n");
                                                return -1;
                                            }
                                            clone1->arguments = carrier->data;
                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "unable to allocate memory\n");
                                                return -1;
                                            }
                                            list_destroy(response1);
                                            return 1;
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

    list_destroy(response1);

    return 0;
}

static int32_t
syntax_item(program_t *program, node_t *node, list_t *response)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = syntax_postfix(program, carrier->base, response1);
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
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                if ((item1->flag & NODE_FLAG_TEMPORARY) != NODE_FLAG_TEMPORARY)
                {
                    syntax_error(program, item1, "unmade class");
                    return -1;
                }
                node_class_t *class1 = (node_class_t *)item1->value;
                node_t *node1 = class1->block;
                node_block_t *block1 = (node_block_t *)node1->value;
                ilist_t *b1;
                for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                {
                    node_t *item2 = (node_t *)b1->value;
                    if (item2->kind == NODE_KIND_FUNC)
                    {
                        node_func_t *func1 = (node_func_t *)item2->value;
                        if (syntax_idstrcmp(func1->key, "[]") == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nds2 = carrier->data;
                            int32_t r2 = syntax_eqaul_psas(program, nps1, nds2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                ilist_t *il1 = list_rpush(response, func1->result);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "unable to allocate memory\n");
                                    return -1;
                                }
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_ARRAY)
            {
                node_block_t *block1 = (node_block_t *)item1->value;
                ilist_t *b1;
                for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                {
                    node_t *item2 = (node_t *)a1->value;
                    ilist_t *il1 = list_rpush(response, item2);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "unable to allocate memory\n");
                        return -1;
                    }
                    break;
                }
            }
            else
            if (item1->kind == NODE_KIND_OBJECT)
            {
                node_block_t *block1 = (node_block_t *)item1->value;
                ilist_t *b1;
                for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                {
                    node_t *item2 = (node_t *)a1->value;
                    node_pair_t *pair2 = (node_pair_t *)item2->value;
                    ilist_t *il1 = list_rpush(response, pair2->value);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "unable to allocate memory\n");
                        return -1;
                    }
                    break;
                }
            }
            else
            {
                syntax_error(program, item1, "not an enumerable");
                return -1;
            }
        }
    }
    return 1;
}

static int32_t
syntax_attribute(program_t *program, node_t *node, list_t *response)
{
    node_binary_t *basic = (node_binary_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = syntax_postfix(program, basic->left, response1);
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
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)item1->value;
                node_t *node2 = class2->block;
                node_block_t *block2 = (node_block_t *)node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class1p = (node_class_t *)item2->value;
                        if (syntax_idcmp(basic->right, class1p->key) == 1)
                        {
                            ilist_t *r2 = list_rpush(response, item2);
                            if (r2 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                            return 1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum1 = (node_enum_t *)item2->value;
                        if (syntax_idcmp(basic->right, enum1->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item2);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                            return 1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FUNC)
                    {
                        node_func_t *func1 = (node_func_t *)item2->value;
                        if (syntax_idcmp(basic->right, func1->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item2);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                            return 1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_PROPERTY)
                    {
                        node_property_t *property1 = (node_property_t *)item2->value;
                        if (syntax_idcmp(basic->right, property1->key) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item2);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                            return 1;
                        }
                    }
                }

                if (class2->heritages != NULL)
                {
                    node_t *node3 = class2->heritages;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    list_t *repository = list_create();
                    if (repository == NULL)
                    {
                        fprintf(stderr, "unable to allocate memory\n");
                        return -1;
                    }
                    list_t *repository_list = list_create();
                    if (repository_list == NULL)
                    {
                        fprintf(stderr, "unable to allocate memory\n");
                        return -1;
                    }

                    int32_t start_new = 0;

                    ilist_t *a3 = block3->list->begin;
                    while (a3 != NULL)
                    {
                        node_t *item3 = (node_t *)a3->value;

                        if (item3->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage3 = (node_heritage_t *)item3->value;

                            list_t *response3 = list_create();
                            if (response3 == NULL)
                            {
                                fprintf(stderr, "unable to allocate memory\n");
                                return -1;
                            }
                            int32_t r3 = syntax_postfix(program, heritage3->type, response3);
                            if (r3 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r3 == 0)
                            {
                                syntax_error(program, heritage3->type, "reference not found");
                                return -1;
                            }
                            else
                            if (r3 == 1)
                            {
                                ilist_t *a4;
                                for (a4 = response3->begin;a4 != response3->end;a4 = a4->next)
                                {
                                    node_t *item4 = (node_t *)a4->value;
                                    if (item4->kind == NODE_KIND_CLASS)
                                    {
                                        node_class_t *class4 = (node_class_t *)item4->value;
                                        node_t *node4 = class4->block;
                                        node_block_t *block4 = (node_block_t *)node4->value;
                                        ilist_t *a4;
                                        for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                        {
                                            node_t *item4 = (node_t *)a4->value;
                                            if (item4->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class1p = (node_class_t *)item4->value;
                                                if (syntax_idcmp(basic->right, class1p->key) == 1)
                                                {
                                                    ilist_t *r1 = list_rpush(response, item4);
                                                    if (r1 == NULL)
                                                    {
                                                        fprintf(stderr, "unable to allocate memory\n");
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum1 = (node_enum_t *)item4->value;
                                                if (syntax_idcmp(basic->right, enum1->key) == 1)
                                                {
                                                    ilist_t *r1 = list_rpush(response, item4);
                                                    if (r1 == NULL)
                                                    {
                                                        fprintf(stderr, "unable to allocate memory\n");
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_FUNC)
                                            {
                                                node_func_t *func1 = (node_func_t *)item4->value;
                                                if (syntax_idcmp(basic->right, func1->key) == 1)
                                                {
                                                    ilist_t *r1 = list_rpush(response, item4);
                                                    if (r1 == NULL)
                                                    {
                                                        fprintf(stderr, "unable to allocate memory\n");
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_PROPERTY)
                                            {
                                                node_property_t *property1 = (node_property_t *)item4->value;
                                                if (syntax_idcmp(basic->right, property1->key) == 1)
                                                {
                                                    ilist_t *r1 = list_rpush(response, item4);
                                                    if (r1 == NULL)
                                                    {
                                                        fprintf(stderr, "unable to allocate memory\n");
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                            }
                                        }

                                        ilist_t *r5 = list_rpush(repository_list, a3);
                                        if (r5 == NULL)
                                        {
                                            fprintf(stderr, "unable to allocate memory\n");
                                            return -1;
                                        }

                                        ilist_t *r6 = list_rpush(repository, node3);
                                        if (r6 == NULL)
                                        {
                                            fprintf(stderr, "unable to allocate memory\n");
                                            return -1;
                                        }

                                        node3 = class4->heritages;
                                        block3 = (node_block_t *)node3->value;
                                        a3 = block3->list->begin;
                                        start_new = 1;
                                        break;
                                    }
                                }
                            }
                        }

                        if (a3 == block2->list->end)
                        {
                            ilist_t *r5 = list_rpop(repository_list);
                            if (r5 != NULL)
                            {
                                a2 = (ilist_t *)r5->value;
                            }
                            ilist_t *r6 = list_rpop(repository);
                            if (r6 != NULL)
                            {
                                node3 = (node_t *)r6->value;
                                block3 = (node_block_t *)node3->value;
                                continue;
                            }
                            break;
                        }
                        else
                        {
                            if (start_new != 1)
                            {
                                a2 = a2->next;
                            }
                            start_new = 0;
                        }
                    }

                    list_destroy(repository_list);
                    list_destroy(repository);
                }
            }
            else
            {
                syntax_error(program, item1, "not accessable");
                return -1;
            }
        }
    }

    list_destroy(response1);

    return 0;
}

static int32_t
syntax_postfix(program_t *program, node_t *node, list_t *response)
{
    if (node->kind == NODE_KIND_ATTRIBUTE)
    {
        return syntax_attribute(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_PSEUDONYM)
    {
        return syntax_pseudonym(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_ITEM)
    {
        return syntax_item(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_CALL)
    {
        return syntax_call(program, node, response);
    }
    else
    {
        return syntax_primary(program, node, response);
    }
    
}

static int32_t
syntax_tilde(program_t *program, node_t *node, list_t *response)
{
    node_unary_t *unary = (node_unary_t *)node->value;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = syntax_expression(program, unary->right, response1);
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
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            ilist_t *il1 = list_rpush(response, item1);
            if (il1 == NULL)
            {
                fprintf(stderr, "unable to allocate memory\n");
                return -1;
            }
            list_destroy(response1);
            return 1;
        }
    }
    list_destroy(response1);
    return 0;
}

static int32_t
syntax_pos(program_t *program, node_t *node, list_t *response)
{
    node_unary_t *unary = (node_unary_t *)node->value;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = syntax_expression(program, unary->right, response1);
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
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            ilist_t *il1 = list_rpush(response, item1);
            if (il1 == NULL)
            {
                fprintf(stderr, "unable to allocate memory\n");
                return -1;
            }
            list_destroy(response1);
            return 1;
        }
    }
    list_destroy(response1);
    return 0;
}

static int32_t
syntax_neg(program_t *program, node_t *node, list_t *response)
{
    node_unary_t *unary = (node_unary_t *)node->value;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = syntax_expression(program, unary->right, response1);
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
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            ilist_t *il1 = list_rpush(response, item1);
            if (il1 == NULL)
            {
                fprintf(stderr, "unable to allocate memory\n");
                return -1;
            }
            list_destroy(response1);
            return 1;
        }
    }
    list_destroy(response1);
    return 0;
}

static int32_t
syntax_not(program_t *program, node_t *node, list_t *response)
{
    node_unary_t *unary = (node_unary_t *)node->value;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = syntax_expression(program, unary->right, response1);
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
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            ilist_t *il1 = list_rpush(response, item1);
            if (il1 == NULL)
            {
                fprintf(stderr, "unable to allocate memory\n");
                return -1;
            }
            list_destroy(response1);
            return 1;
        }
    }
    list_destroy(response1);
    return 0;
}

static int32_t
syntax_typeof(program_t *program, node_t *node, list_t *response)
{
    node_unary_t *unary = (node_unary_t *)node->value;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = syntax_expression(program, unary->right, response1);
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
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            ilist_t *il1 = list_rpush(response, item1);
            if (il1 == NULL)
            {
                fprintf(stderr, "unable to allocate memory\n");
                return -1;
            }
            list_destroy(response1);
            return 1;
        }
    }
    list_destroy(response1);
    return 0;
}

static int32_t
syntax_sizeof(program_t *program, node_t *node, list_t *response)
{
    node_t *node2 = node_create(node->scope, node->parent, node->position);
	if (node2 == NULL)
	{
		return NULL;
	}
    node2->flag = NODE_FLAG_TEMPORARY;

	node_t *node3 = node_make_number(node2, NULL);
	if (!node3)
	{
		return NULL;
	}

    ilist_t *r1 = list_rpush(response, node3);
    if (r1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }
    return 1;
}

static int32_t
syntax_prefix(program_t *program, node_t *node, list_t *response)
{
    if (node->kind == NODE_KIND_TILDE)
    {
        return syntax_tilde(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_POS)
    {
        return syntax_pos(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_NEG)
    {
        return syntax_neg(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_TYPEOF)
    {
        return syntax_typeof(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_SIZEOF)
    {
        return syntax_sizeof(program, node, response);
    }
    else
    {
        return syntax_postfix(program, node, response);
    }
}

static int32_t
syntax_power(program_t *program, node_t *node, list_t *response)
{
    node_binary_t *binary = (node_binary_t *)node->value;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "unable to allocate memory\n");
        return -1;
    }

    int32_t r1 = 0;
    if (binary->left->kind == NODE_KIND_POW)
    {
        r1 = syntax_power(program, binary->left, response1);
    }
    else
    {
        r1 = syntax_prefix(program, binary->left, response1);
    }
    
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        syntax_error(program, binary->left, "reference not found");
        return -1;
    }
    else
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            ilist_t *il1 = list_rpush(response, item1);
            if (il1 == NULL)
            {
                fprintf(stderr, "unable to allocate memory\n");
                return -1;
            }
            list_destroy(response1);
            return 1;
        }
    }
    list_destroy(response1);
    return 0;
}


static int32_t
syntax_expression(program_t *program, node_t *node, list_t *response)
{
    return 1;
}

static int32_t
syntax_if(program_t *program, node_t *node)
{
    node_if_t *if1 = (node_if_t *)node->value;

    if (if1->key != NULL)
    {
        node_t *node_current = node->parent;
        while (node_current != NULL)
        {
            if (node_current->kind == NODE_KIND_BODY)
            {
                node_block_t *block2 = (node_block_t *)node_current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_idcmp(if1->key, if2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                            if (syntax_idcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(if1->key, var2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(if1->key, property1->key) == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item3->position.line, item3->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(if1->key, property1->type) == 1)
                                            {
                                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                    item3->position.line, item3->position.column);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                node_current = node_current->parent;
            }
            else
            if (node_current->kind == NODE_KIND_FUNC)
            {
                node_func_t *func2 = (node_func_t *)node_current->value;
                if (func2->generics != NULL)
                {
                    node_t *node2 = func2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->id == node->id)
                        {
                            break;
                        }

                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (syntax_idcmp(if1->key, generic3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }

                if (func2->parameters != NULL)
                {
                    node_t *node3 = func2->parameters;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        if (item3->id == node->id)
                        {
                            break;
                        }

                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (syntax_idcmp(if1->key, parameter3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item3->position.line, item3->position.column);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                node_current = node_current->parent;
            }
        }
    }

	return 1;
}

static int32_t
syntax_for(program_t *program, node_t *node)
{
    node_for_t *for1 = (node_for_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *node_current = node->parent;
        while (node_current != NULL)
        {
            if (node_current->kind == NODE_KIND_BODY)
            {
                node_block_t *block2 = (node_block_t *)node_current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_idcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                            if (syntax_idcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(for1->key, var2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(for1->key, property1->key) == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item3->position.line, item3->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(for1->key, property1->type) == 1)
                                            {
                                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                    item3->position.line, item3->position.column);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                node_current = node_current->parent;
            }
            else
            if (node_current->kind == NODE_KIND_FUNC)
            {
                node_func_t *func2 = (node_func_t *)node_current->value;

                if (func2->generics != NULL)
                {
                    node_t *node2 = func2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->id == node->id)
                        {
                            break;
                        }

                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (syntax_idcmp(for1->key, generic3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }

                if (func2->parameters != NULL)
                {
                    node_t *node3 = func2->parameters;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        if (item3->id == node->id)
                        {
                            break;
                        }

                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (syntax_idcmp(for1->key, parameter3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item3->position.line, item3->position.column);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                node_current = node_current->parent;
            }
        }
    }
	return 1;
}

static int32_t
syntax_forin(program_t *program, node_t *node)
{
    node_forin_t *for1 = (node_forin_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *node_current = node->parent;
        while (node_current != NULL)
        {
            if (node_current->kind == NODE_KIND_BODY)
            {
                node_block_t *block2 = (node_block_t *)node_current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_idcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                            if (syntax_idcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(for1->key, var2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(for1->key, property1->key) == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item3->position.line, item3->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(for1->key, property1->type) == 1)
                                            {
                                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                    item3->position.line, item3->position.column);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                node_current = node_current->parent;
            }
            else
            if (node_current->kind == NODE_KIND_FUNC)
            {
                node_func_t *func2 = (node_func_t *)node_current->value;

                if (func2->generics != NULL)
                {
                    node_t *node2 = func2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->id == node->id)
                        {
                            break;
                        }

                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (syntax_idcmp(for1->key, generic3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }

                if (func2->parameters != NULL)
                {
                    node_t *node3 = func2->parameters;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        if (item3->id == node->id)
                        {
                            break;
                        }

                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (syntax_idcmp(for1->key, parameter3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item3->position.line, item3->position.column);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                node_current = node_current->parent;
            }
        }
    }
    return 1;
}

static int32_t
syntax_parameter(program_t *program, node_t *node)
{
    node_parameter_t *parameter1 = (node_parameter_t *)node->value;
    
    node_t *node_current = node->parent;
    while (node_current != NULL)
    {
        if (node_current->kind == NODE_KIND_PARAMETERS)
        {
            node_block_t *block2 = (node_block_t *)node_current->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == node->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter3 = (node_parameter_t *)item2->value;
                    if (syntax_idcmp(parameter1->key, parameter3->key) == 1)
                    {
                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
            }
            node_current = node_current->parent;
        }
        else
        if (node_current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)node_current->value;

            if (class2->generics != NULL)
            {
                node_t *node2 = class2->generics;
                node_block_t *block3 = (node_block_t *)node2->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (syntax_idcmp(parameter1->key, generic3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
                            return -1;
                        }
                    }
                }
            }
            break;
        }
        else
        if (node_current->kind == NODE_KIND_FUNC)
        {
            node_func_t *func2 = (node_func_t *)node_current->value;

            if (func2->generics != NULL)
            {
                node_t *node2 = func2->generics;
                node_block_t *block3 = (node_block_t *)node2->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (syntax_idcmp(parameter1->key, generic3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
                            return -1;
                        }
                    }
                }
            }
            break;
        }
        else
        {
            node_current = node_current->parent;
        }
    }

    return 1;
}

static int32_t
syntax_parameters(program_t *program, node_t *node)
{
	node_block_t *parameters = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = parameters->list->begin;a1 != parameters->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = syntax_parameter(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
syntax_catch(program_t *program, node_t *node)
{
    return 1;
}

static int32_t
syntax_try(program_t *program, node_t *node)
{
	return 1;
}

static int32_t
syntax_var(program_t *program, node_t *node)
{
    node_var_t *var1 = (node_var_t *)node->value;

    node_t *node_current = node->parent;
    while (node_current != NULL)
    {
        if (node_current->kind == NODE_KIND_BODY)
        {
            node_block_t *block2 = (node_block_t *)node_current->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == node->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_IF)
                {
                    node_if_t *if2 = (node_if_t *)item2->value;
                    if (if2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(var1->key, if2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(property1->key, if2->key) == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item3->position.line, item3->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(property1->type, if2->key) == 1)
                                            {
                                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                    item3->position.line, item3->position.column);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for2 = (node_for_t *)item2->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(var1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(property1->key, for2->key) == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item3->position.line, item3->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(property1->type, for2->key) == 1)
                                            {
                                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                    item3->position.line, item3->position.column);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
                                            return -1;
                                        }
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
                            if (syntax_idcmp(var1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(property1->key, for2->key) == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item3->position.line, item3->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(property1->type, for2->key) == 1)
                                            {
                                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                    item3->position.line, item3->position.column);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
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
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(var1->key, var2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(property1->key, var2->key) == 1)
                                        {
                                            syntax_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(property1->type, var2->key) == 1)
                                            {
                                                syntax_error(program, property1->type, "already defined, previous in (%lld:%lld)",
                                                    item2->position.line, item2->position.column);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
                                            return -1;
                                        }
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(var1->key, property1->key) == 1)
                                        {
                                            syntax_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                                property1->key->position.line, property1->key->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(var1->key, property1->type) == 1)
                                            {
                                                syntax_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                                    property1->type->position.line, property1->type->position.column);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
                                            return -1;
                                        }
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    node_t *node4 = (node_t *)var2->key;
                                    node_block_t *block4 = (node_block_t *)node4->value;
                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_PROPERTY)
                                        {
                                            node_property_t *property2 = (node_property_t *)item4->value;
                                            if (property1->type == NULL)
                                            {
                                                if (property2->type == NULL)
                                                {
                                                    if (syntax_idcmp(property1->key, property2->key) == 1)
                                                    {
                                                        syntax_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                                            property2->key->position.line, property2->key->position.column);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                {
                                                    if (property1->type->kind == NODE_KIND_ID)
                                                    {
                                                        if (property2->type->kind == NODE_KIND_ID)
                                                        {
                                                            if (syntax_idcmp(property1->type, property2->type) == 1)
                                                            {
                                                                syntax_error(program, property1->type, "already defined, previous in (%lld:%lld)",
                                                                    property2->type->position.line, property2->type->position.column);
                                                                return -1;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            syntax_error(program, property2->type, "not an identifier");
                                                            return -1;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        syntax_error(program, property1->type, "not an identifier");
                                                        return -1;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (property2->type == NULL)
                                                {
                                                    if (property1->type->kind == NODE_KIND_ID)
                                                    {
                                                        if (syntax_idcmp(property1->type, property2->key) == 1)
                                                        {
                                                            syntax_error(program, property1->type, "already defined, previous in (%lld:%lld)",
                                                                property2->key->position.line, property2->key->position.column);
                                                            return -1;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        syntax_error(program, property1->type, "not an identifier");
                                                        return -1;
                                                    }
                                                }
                                                else
                                                {
                                                    if (property1->type->kind == NODE_KIND_ID)
                                                    {
                                                        if (property2->type->kind == NODE_KIND_ID)
                                                        {
                                                            if (syntax_idcmp(property1->type, property2->type) == 1)
                                                            {
                                                                syntax_error(program, property1->type, "already defined, previous in (%lld:%lld)",
                                                                    property2->type->position.line, property2->type->position.column);
                                                                return -1;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            syntax_error(program, property2->type, "not an identifier");
                                                            return -1;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        syntax_error(program, property1->type, "not an identifier");
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
            }
            node_current = node_current->parent;
        }
        else
        if (node_current->kind == NODE_KIND_FUNC)
        {
            node_func_t *func2 = (node_func_t *)node_current->value;

            if (func2->generics != NULL)
            {
                node_t *node2 = func2->generics;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item2->value;
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(var1->key, generic3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(property1->key, generic3->key) == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item3->position.line, item3->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind != NODE_KIND_ID)
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
                                            return -1;
                                        }
                                        if (syntax_idcmp(property1->type, generic3->key) == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item3->position.line, item3->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (func2->parameters != NULL)
            {
                node_t *node3 = func2->parameters;
                node_block_t *block3 = (node_block_t *)node3->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(var1->key, parameter3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item3->position.line, item3->position.column);
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
                                if (item4->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item4->value;
                                    if (property1->type == NULL)
                                    {
                                        if (syntax_idcmp(property1->key, parameter3->key) == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item4->position.line, item4->position.column);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        if (property1->type->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(property1->type, parameter3->key) == 1)
                                            {
                                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                    item4->position.line, item4->position.column);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            syntax_error(program, property1->type, "not an identifier");
                                            return -1;
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
        {
            node_current = node_current->parent;
        }
    }

	return 1;
}

static int32_t
syntax_statement(program_t *program, node_t *node)
{
    if (node->kind == NODE_KIND_IF)
    {
        int32_t result;
        result = syntax_if(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else 
    if (node->kind == NODE_KIND_FOR)
    {
        int32_t result;
        result = syntax_for(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_FORIN)
    {
        int32_t result;
        result = syntax_forin(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_TRY)
    {
        int32_t result;
        result = syntax_try(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t result;
        result = syntax_var(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    return 1;
}

static int32_t
syntax_body(program_t *program, node_t *node)
{
    node_block_t *block1 = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        int32_t result = syntax_statement(program, item);
        if (result == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
syntax_generic(program_t *program, node_t *node)
{
    node_generic_t *generic1 = (node_generic_t *)node->value;
    
    node_t *node_current = node->parent;
    while (node_current != NULL)
    {
        if (node_current->kind == NODE_KIND_GENERICS)
        {
            node_block_t *block2 = (node_block_t *)node_current->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == node->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic3 = (node_generic_t *)item2->value;
                    if (syntax_idcmp(generic1->key, generic3->key) == 1)
                    {
                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
            }
            node_current = node_current->parent;
        }
        else
        if (node_current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)node_current->value;
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
                    if (syntax_idcmp(generic1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *b3;
                            for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                            {
                                node_t *item3 = (node_t *)b3->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                                node->position.line, node->position.column);
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

                    if (syntax_idcmp(generic1->key, func2->key) == 1)
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
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (syntax_idcmp(generic1->key, enum2->key) == 1)
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
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(generic1->key, property2->key) == 1)
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
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        else
        {
            node_current = node_current->parent;
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
        int32_t result;
        result = syntax_generic(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
syntax_func(program_t *program, node_t *node)
{
	node_func_t *func1 = (node_func_t *)node->value;
    node_t *node1 = func1->body;
    node_block_t *block1 = (node_block_t *)node1->value;

    node_t *node_current = node->parent;
    while (node_current != NULL)
    {
        if (node_current->kind == NODE_KIND_MODULE)
        {
            node_module_t *module2 = (node_module_t *)node_current->value;
            node_t *node2 = module2->block;
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
                    if (syntax_idcmp(func1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *b3;
                            for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                            {
                                node_t *item3 = (node_t *)b3->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        else
        if (node_current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)node_current->value;
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
                    if (syntax_idcmp(func1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *b3;
                            for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                            {
                                node_t *item3 = (node_t *)b3->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
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
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(func1->key, property2->key) == 1)
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
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        else
        {
            node_current = node_current->parent;
        }
    }

    if (func1->generics != NULL)
    {
        int32_t r1 = syntax_generics(program, func1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (func1->parameters != NULL)
    {
        int32_t r1 = syntax_parameters(program, func1->parameters);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (func1->body != NULL)
    {
        int32_t r1 = syntax_body(program, func1->body);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
syntax_enum(program_t *program, node_t *node)
{
	node_enum_t *enum1 = (node_enum_t *)node->value;
    node_t *node1 = enum1->block;
    node_block_t *block1 = (node_block_t *)node1->value;
    
    node_t *node_current = node->parent;
    while (node_current != NULL)
    {
        if (node_current->kind == NODE_KIND_MODULE)
        {
            node_module_t *module2 = (node_module_t *)node_current->value;
            node_t *node2 = module2->block;
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
                    if (syntax_idcmp(enum1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *b3;
                            for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                            {
                                node_t *item3 = (node_t *)b3->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                if (item2->kind == NODE_KIND_ENUM)
                {
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
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        else
        if (node_current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)node_current->value;
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
                    if (syntax_idcmp(enum1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *b3;
                            for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                            {
                                node_t *item3 = (node_t *)b3->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                if (item2->kind == NODE_KIND_ENUM)
                {
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
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func2 = (node_func_t *)item2->value;

                    if (syntax_idcmp(enum1->key, func2->key) == 1)
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
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(enum1->key, property2->key) == 1)
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
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        else
        {
            node_current = node_current->parent;
        }
    }

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
syntax_import(program_t *program, node_t *node)
{
	//node_import_t *node_import = (node_import_t *)node->value;
	return 1;
}

static int32_t
syntax_property(program_t *program, node_t *node)
{
    node_property_t *property1 = (node_property_t *)node->value;
    
    node_t *node_current = node->parent;
    while (node_current != NULL)
    {
        if (node_current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)node_current->value;

            if (class2->generics != NULL)
            {
                node_t *node2 = class2->generics;
                node_block_t *block3 = (node_block_t *)node2->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (syntax_idcmp(property1->key, generic3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
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
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_HERITAGE)
                    {
                        node_heritage_t *heritage3 = (node_heritage_t *)item3->value;
                        if (syntax_idcmp(property1->key, heritage3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
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
                    if (syntax_idcmp(property1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *b3;
                            for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                            {
                                node_t *item3 = (node_t *)b3->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                                node->position.line, node->position.column);
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

                    if (syntax_idcmp(property1->key, func2->key) == 1)
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
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
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
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
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
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        else
        {
            node_current = node_current->parent;
        }
    }
    return 1;
}

static int32_t
syntax_heritage(program_t *program, node_t *node)
{
    node_heritage_t *heritage1 = (node_heritage_t *)node->value;
    
    node_t *node_current = node->parent;
    while (node_current != NULL)
    {
        if (node_current->kind == NODE_KIND_HERITAGES)
        {
            node_block_t *block2 = (node_block_t *)node_current->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == node->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_HERITAGE)
                {
                    node_heritage_t *heritage3 = (node_heritage_t *)item2->value;
                    if (syntax_idcmp(heritage1->key, heritage3->key) == 1)
                    {
                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
            }
            node_current = node_current->parent;
        }
        else
        if (node_current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)node_current->value;

            if (class2->generics != NULL)
            {
                node_t *node2 = class2->generics;
                node_block_t *block3 = (node_block_t *)node2->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (syntax_idcmp(heritage1->key, generic3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
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
                    if (syntax_idcmp(heritage1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *b3;
                            for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                            {
                                node_t *item3 = (node_t *)b3->value;
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                                node->position.line, node->position.column);
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

                    if (syntax_idcmp(heritage1->key, func2->key) == 1)
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
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (syntax_idcmp(heritage1->key, enum2->key) == 1)
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
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(heritage1->key, property2->key) == 1)
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
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        else
        {
            node_current = node_current->parent;
        }
    }

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
        int32_t result;
        result = syntax_heritage(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
syntax_class(program_t *program, node_t *node)
{
	node_class_t *class1 = (node_class_t *)node->value;
    node_t *node1 = class1->block;
    node_block_t *block1 = (node_block_t *)node1->value;

    node_t *node_current = node->parent;
    while (node_current != NULL)
    {
        if (node_current->kind == NODE_KIND_MODULE)
        {
            node_module_t *module2 = (node_module_t *)node_current->value;
            node_t *node2 = module2->block;
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
                    if (syntax_idcmp(class1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
                                    if (syntax_idstrcmp(func1->key, "constructor") == 1)
                                    {
                                        node_t *node3 = class3->block;
                                        node_block_t *block3 = (node_block_t *)node3->value;
                                        ilist_t *b3;
                                        for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                                        {
                                            node_t *item3 = (node_t *)b3->value;
                                            if (item3->kind == NODE_KIND_FUNC)
                                            {
                                                node_func_t *func3 = (node_func_t *)item3->value;
                                                if (syntax_idstrcmp(func3->key, "constructor") == 1)
                                                {
                                                    node_t *nps1 = func1->parameters;
                                                    node_t *nps2 = func3->parameters;
                                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                                    if (r2 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r2 == 1)
                                                    {
                                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (syntax_idcmp(class1->key, enum2->key) == 1)
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
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
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
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
            break;
        }
        else
        if (node_current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)node_current->value;
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
                    if (syntax_idcmp(class1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
                                    if (syntax_idstrcmp(func1->key, "constructor") == 1)
                                    {
                                        node_t *node3 = class3->block;
                                        node_block_t *block3 = (node_block_t *)node3->value;
                                        ilist_t *b3;
                                        for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                                        {
                                            node_t *item3 = (node_t *)b3->value;
                                            if (item3->kind == NODE_KIND_FUNC)
                                            {
                                                node_func_t *func3 = (node_func_t *)item3->value;
                                                if (syntax_idstrcmp(func3->key, "constructor") == 1)
                                                {
                                                    node_t *nps1 = func1->parameters;
                                                    node_t *nps2 = func3->parameters;
                                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                                    if (r2 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r2 == 1)
                                                    {
                                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func2 = (node_func_t *)item2->value;

                    if (syntax_idcmp(class1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
                                    if (syntax_idstrcmp(func1->key, "constructor") == 1)
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
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (syntax_idcmp(class1->key, enum2->key) == 1)
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
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
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
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(class1->key, property2->key) == 1)
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
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
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
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
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
            break;
        }
        else
        {
            node_current = node_current->parent;
        }
    }

    if (class1->generics != NULL)
    {
        int32_t r1 = syntax_generics(program, class1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->heritages != NULL)
    {
        int32_t r1 = syntax_heritages(program, class1->heritages);
        if (r1 == -1)
        {
            return -1;
        }
    }

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
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

	return 1;
}

static int32_t
syntax_module(program_t *program, node_t *node)
{
	node_module_t *module = (node_module_t *)node->value;
    node_t *node1 = module->block;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin; a1 != block1->list->end; a1 = a1->next)
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
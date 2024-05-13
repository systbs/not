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
#include "../../container/stack.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../program.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../syntax/syntax.h"
#include "../error.h"
#include "../opcode.h"

error_t *
intermediate_error(program_t *program, node_t *node, const char *format, ...)
{
	char *message = malloc(1024);
	if (message == NULL)
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

	error_t *error = error_create(node->position, message);
	if (error == NULL)
	{
		return NULL;
	}

    error->origin = node;

	if (list_rpush(program->errors, error))
	{
		return NULL;
	}

	return error;
}

int32_t
intermediate_idcmp(node_t *n1, node_t *n2)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	node_basic_t *nb2 = (node_basic_t *)n2->value;

	//printf("%s %s\n", nb1->value, nb2->value);

	return (strcmp(nb1->value, nb2->value) == 0);
}

int32_t
intermediate_idstrcmp(node_t *n1, char *name)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}


static int32_t
intermediate_var(program_t *program, node_t *node, uint64_t flag);

static int32_t
intermediate_parameter(program_t *program, node_t *node, uint64_t flag);

static int32_t
intermediate_parameters(program_t *program, node_t *node, uint64_t flag);

static int32_t
intermediate_generics(program_t *program, node_t *node, uint64_t flag);

static int32_t
intermediate_body(program_t *program, node_t *node, uint64_t flag);

int32_t
intermediate_module(program_t *program, node_t *node, uint64_t flag);

static int32_t
intermediate_expression(program_t *program, node_t *node, uint64_t flag)
{
    return 1;
}

static int32_t
intermediate_assign(program_t *program, node_t *node, uint64_t flag)
{
    return 1;
}

static int32_t
intermediate_if(program_t *program, node_t *node, uint64_t flag)
{
    node_if_t *if1 = (node_if_t *)node->value;

    int32_t r1 = intermediate_expression(program, if1->condition, flag);
    if (r1 == -1)
    {
        return -1;
    }

    istack_t *isk1 = stack_rpush(program->repository, OPCODE_JZ);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk1 = stack_rpush(program->repository, OPCODE_NOP);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    if (if1->then_body != NULL)
    {
        int32_t r2 = intermediate_body(program, if1->then_body, flag);
        if (r2 == -1)
        {
            return -1;
        }
    }

    if (if1->else_body != NULL)
    {
        node_t *else_body1 = if1->else_body;

        istack_t *isk2 = stack_rpush(program->repository, OPCODE_JMP);
        if (isk2 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk2 = stack_rpush(program->repository, OPCODE_NOP);
        if (isk2 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        if (else_body1->kind == NODE_KIND_IF)
        {
            istack_t *isk3 = stack_rpush(program->repository, OPCODE_NOP);
            if (isk3 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }
            isk1->value = (int64_t)isk3;
            isk2->value = (int64_t)isk3;

            int32_t r2 = intermediate_if(program, if1->else_body, flag);
            if (r2 == -1)
            {
                return -1;
            }
        }
        else
        {
            int32_t r2 = intermediate_body(program, if1->else_body, flag);
            if (r2 == -1)
            {
                return -1;
            }
        }

        isk1->value = (int64_t)isk2->next;
        isk1 = isk2;
    }

    istack_t *isk2 = stack_rpush(program->repository, OPCODE_NOP);
    if (isk2 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    isk1->value = (int64_t)isk2;

	return 1;
}

static int32_t
intermediate_for(program_t *program, node_t *node, uint64_t flag)
{
    node_for_t *for1 = (node_for_t *)node->value;

    for1->n_initiator = program->n_variables - 1;

    istack_t *isk1 = stack_rpush(program->repository_nvars, program->n_variables);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk2 = stack_rpush(program->repository, OPCODE_ENT);
    if (isk2 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    for1->entry_condition = isk2;

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
                int32_t r1 = intermediate_var(program, item1, flag);
                if (r1 == -1)
                {
                    return -1;
                }
            }
            else
            {
                int32_t r1 = intermediate_assign(program, item1, flag);
                if (r1 == -1)
                {
                    return -1;
                }
            }
        }
    }

    istack_t *isk3 = stack_rpush(program->repository, OPCODE_NOP);
    if (isk3 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    for1->entry_condition = isk3;

    isk3 = stack_rpush(program->repository, OPCODE_NOP);
    if (isk3 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    for1->exit = isk3;

    int32_t r1 = intermediate_expression(program, for1->condition, flag);
    if (r1 == -1)
    {
        return -1;
    }

    istack_t *isk4 = stack_rpush(program->repository, OPCODE_JZ);
    if (isk4 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk4 = stack_rpush(program->repository, OPCODE_NOP);
    if (isk4 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    if (for1->body != NULL)
    {
        int32_t r2 = intermediate_body(program, for1->body, flag);
        if (r2 == -1)
        {
            return -1;
        }
    }

    if (for1->incrementor != NULL)
    {
        node_t *node1 = for1->incrementor;
        node_block_t *block1 = node1->value;

        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            int32_t r2 = intermediate_assign(program, item1, flag);
            if (r2 == -1)
            {
                return -1;
            }
        }
    }

    istack_t *isk5 = stack_rpush(program->repository, OPCODE_LEV);
    if (isk5 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk4->value = (int64_t)isk5;
    
    stack_unlink(program->repository, isk3);
    stack_link(program->repository, isk5, isk3);

    for1->n_variables = program->n_variables - for1->n_initiator;

    istack_t *isk6 = stack_insert(program->repository, isk2->next, for1->n_initiator);
    if (isk6 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk6 = stack_insert(program->repository, isk2->next, for1->n_variables);
    if (isk6 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk7 = stack_rpop(program->repository_nvars);
    if (isk7 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tThe stack is empty\n", __FILE__, __LINE__);
        return -1;
    }
    program->n_variables = isk7->value;

	return 1;
}

static int32_t
intermediate_parameter(program_t *program, node_t *node, uint64_t flag)
{
    node_parameter_t *parameter1 = (node_parameter_t *)node->value;
    
    parameter1->n = program->n_variables;
    program->n_variables += 1;

    istack_t *isk1 = stack_rpush(program->repository, OPCODE_IMM);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk1 = stack_rpush(program->repository, parameter1->n);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk1 = stack_rpush(program->repository, OPCODE_PSAVE);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    return 1;
}

static int32_t
intermediate_parameters(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *parameters = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = parameters->list->begin;a1 != parameters->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result = intermediate_parameter(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
intermediate_generic(program_t *program, node_t *node, uint64_t flag)
{
    node_generic_t *generic1 = (node_generic_t *)node->value;
    
    generic1->n = program->n_variables;
    program->n_variables += 1;

    istack_t *isk1 = stack_rpush(program->repository, OPCODE_IMM);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk1 = stack_rpush(program->repository, generic1->n);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk1 = stack_rpush(program->repository, OPCODE_PSAVE);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    return 1;
}

static int32_t
intermediate_generics(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *block1 = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t r1 = intermediate_generic(program, item1, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
intermediate_catch(program_t *program, node_t *node, uint64_t flag)
{
    node_catch_t *catch1 = (node_catch_t *)node->value;

    catch1->n_initiator = program->n_variables - 1;

    istack_t *isk1 = stack_rpush(program->repository_nvars, program->n_variables);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk2 = stack_rpush(program->repository, OPCODE_CATCH);
    if (isk2 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk2 = stack_rpush(program->repository, (int64_t)catch1);
    if (isk2 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk3 = stack_rpush(program->repository, OPCODE_JMP);
    if (isk3 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk3 = stack_rpush(program->repository, OPCODE_NOP);
    if (isk3 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk4 = stack_rpush(program->repository, OPCODE_ENT);
    if (isk4 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    catch1->entry = isk4;

    if (catch1->parameters != NULL)
    {
        int32_t r1 = intermediate_parameters(program, catch1->parameters, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (catch1->body != NULL)
    {
        int32_t r1 = intermediate_body(program, catch1->body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    istack_t *isk5 = stack_rpush(program->repository, OPCODE_LEV);
    if (isk5 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk6 = stack_rpush(program->repository, OPCODE_NOP);
    if (isk6 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk3->value = (int64_t)isk6;

    catch1->n_variables = program->n_variables - catch1->n_initiator;

    istack_t *isk7 = stack_insert(program->repository, isk4->next, catch1->n_initiator);
    if (isk7 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk7 = stack_insert(program->repository, isk4->next, catch1->n_variables);
    if (isk7 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk8 = stack_rpop(program->repository_nvars);
    if (isk8 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tThe stack is empty\n", __FILE__, __LINE__);
        return -1;
    }
    program->n_variables = isk8->value;

    return 1;
}

static int32_t
intermediate_try(program_t *program, node_t *node, uint64_t flag)
{
	node_try_t *try1 = (node_try_t *)node->value;

    istack_t *isk1 = stack_rpush(program->repository, OPCODE_TENT);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    if (try1->catchs != NULL)
    {
        node_t *node1 = try1->catchs;
        node_block_t *block1 = (node_block_t *)node1->value;

        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            int32_t r1 = intermediate_catch(program, item1, flag);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }

    if (try1->body != NULL)
    {
        int32_t r1 = intermediate_body(program, try1->body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    istack_t *isk2 = stack_rpush(program->repository, OPCODE_TLEV);
    if (isk2 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

	return 1;
}

static int32_t
intermediate_entity(program_t *program, node_t *node, uint64_t flag)
{
    node_entity_t *entity1 = (node_entity_t *)node->value;

    entity1->n = program->n_variables;
    program->n_variables += 1;

    if (entity1->type != NULL)
    {
        if (entity1->value != NULL)
        {
            int32_t r1 = intermediate_expression(program, entity1->value, flag);
            if (r1 == -1)
            {
                return -1;
            }
        }

        istack_t *isk1 = stack_rpush(program->repository, OPCODE_LS);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, (int64_t)entity1->type);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_PUSH);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_IMM);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, entity1->n);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_PSAVE);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        if (entity1->value != NULL)
        {
            int32_t r1 = intermediate_expression(program, entity1->value, flag);
            if (r1 == -1)
            {
                return -1;
            }
        }

        istack_t *isk1 = stack_rpush(program->repository, OPCODE_LS);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, (int64_t)entity1->key);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_PUSH);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_IMM);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, entity1->n);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_PSAVE);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
    }

    return 1;
}

static int32_t
intermediate_set(program_t *program, node_t *node, uint64_t flag)
{
    node_block_t *block1 = (node_block_t *)node->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t r1 = intermediate_entity(program, item1, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
intermediate_var(program_t *program, node_t *node, uint64_t flag)
{
    node_var_t *var1 = (node_var_t *)node->value;

    if (var1->key->kind == NODE_KIND_SET)
    {
        int32_t r1 = intermediate_expression(program, var1->value, flag);
        if (r1 == -1)
        {
            return -1;
        }

        istack_t *isk1 = stack_rpush(program->repository, OPCODE_PUSH);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r2 = intermediate_set(program, var1->key, flag);
        if (r2 == -1)
        {
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_POP);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        var1->n = program->n_variables;
        program->n_variables += 1;

        if (var1->value != NULL)
        {
            int32_t r1 = intermediate_expression(program, var1->value, flag);
            if (r1 == -1)
            {
                return -1;
            }

            istack_t *isk1 = stack_rpush(program->repository, OPCODE_PUSH);
            if (isk1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }

            isk1 = stack_rpush(program->repository, OPCODE_IMM);
            if (isk1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }

            isk1 = stack_rpush(program->repository, var1->n);
            if (isk1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }

            isk1 = stack_rpush(program->repository, OPCODE_PSAVE);
            if (isk1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
intermediate_argument(program_t *program, node_t *node, uint64_t flag)
{
    node_argument_t *argument1 = (node_argument_t *)node->value;
    
    if (argument1->value != NULL)
    {
        int32_t r1 = intermediate_expression(program, argument1->value, flag);
        if (r1 == -1)
        {
            return -1;
        }

        istack_t *isk1 = stack_rpush(program->repository, OPCODE_LABEL);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, (int64_t)argument1->key);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_PUSH);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        int32_t r1 = intermediate_expression(program, argument1->value, flag);
        if (r1 == -1)
        {
            return -1;
        }

        istack_t *isk1 = stack_rpush(program->repository, OPCODE_PUSH);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
    }

    return 1;
}

static int32_t
intermediate_arguments(program_t *program, node_t *node, uint64_t flag)
{
    int64_t n = 0;
	node_block_t *block1 = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t r1 = intermediate_argument(program, item1, flag);
        if (r1 == -1)
        {
            return -1;
        }

        n += 1;
    }

    istack_t *isk1 = stack_rpush(program->repository, OPCODE_PACK);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk1 = stack_rpush(program->repository, n);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

	return 1;
}

static int32_t
intermediate_throw(program_t *program, node_t *node, uint64_t flag)
{
    node_throw_t *throw1 = (node_throw_t *)node->value;
    
    int32_t r1 = intermediate_arguments(program, throw1->arguments, flag);
    if (r1 == -1)
    {
        return -1;
    }

    istack_t *isk1 = stack_rpush(program->repository, OPCODE_THROW);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    return 1;
}

static int32_t
intermediate_return(program_t *program, node_t *node, uint64_t flag)
{
    node_unary_t *unary1 = (node_unary_t *)node->value;

    if (unary1->right != NULL)
    {
        int32_t r1 = intermediate_expression(program, unary1->right, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    istack_t *isk1 = stack_rpush(program->repository, OPCODE_RET);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    return 1;
}

static int32_t
intermediate_continue(program_t *program, node_t *node, uint64_t flag)
{
    node_unary_t *unary1 = (node_unary_t *)node->value;
    
    node_t *current1 = (node_t *)node->parent;
    while (current1 != NULL)
    {
        if (current1->kind == NODE_KIND_FOR)
        {
            if (unary1->right != NULL)
            {
                node_for_t *for1 = (node_for_t *)current1->value;
                if (for1->key != NULL)
                {
                    if (intermediate_idcmp(for1->key, unary1->right) == 1)
                    {
                        break;
                    }
                }
            }
            else
            {
                break;
            }
        }
        current1 = current1->parent;
    }

    if (current1 != NULL)
    {
        node_for_t *for1 = (node_for_t *)current1->value;

        istack_t *isk1 = stack_rpush(program->repository, OPCODE_JMP);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, (int64_t)for1->entry_condition);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
    }

    return 1;
}

static int32_t
intermediate_break(program_t *program, node_t *node, uint64_t flag)
{
    node_unary_t *unary1 = (node_unary_t *)node->value;
    
    node_t *current1 = (node_t *)node->parent;
    while (current1 != NULL)
    {
        if (current1->kind == NODE_KIND_FOR)
        {
            if (unary1->right != NULL)
            {
                node_for_t *for1 = (node_for_t *)current1->value;
                if (for1->key != NULL)
                {
                    if (intermediate_idcmp(for1->key, unary1->right) == 1)
                    {
                        break;
                    }
                }
            }
            else
            {
                break;
            }
        }
        current1 = current1->parent;
    }

    if (current1 != NULL)
    {
        node_for_t *for1 = (node_for_t *)current1->value;

        istack_t *isk1 = stack_rpush(program->repository, OPCODE_JMP);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, (int64_t)for1->exit);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
    }

    return 1;
}

static int32_t
intermediate_statement(program_t *program, node_t *node, uint64_t flag)
{
    if (node->kind == NODE_KIND_IF)
    {
        int32_t result = intermediate_if(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else 
    if (node->kind == NODE_KIND_FOR)
    {
        int32_t result = intermediate_for(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_TRY)
    {
        int32_t result = intermediate_try(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t result = intermediate_var(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_THROW)
    {
        int32_t result = intermediate_throw(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_RETURN)
    {
        int32_t result = intermediate_return(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_CONTINUE)
    {
        int32_t result = intermediate_continue(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_BREAK)
    {
        int32_t result = intermediate_break(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    {
        return intermediate_assign(program, node, flag);
    }
    return 1;
}

static int32_t
intermediate_body(program_t *program, node_t *node, uint64_t flag)
{
    node_body_t *body1 = (node_body_t *)node->value;

    body1->n_initiator = program->n_variables - 1;

    istack_t *isk1 = stack_rpush(program->repository_nvars, program->n_variables);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk2 = stack_rpush(program->repository, OPCODE_ENT);
    if (isk2 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    ilist_t *a1;
    for (a1 = body1->list->begin;a1 != body1->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        int32_t result = intermediate_statement(program, item, flag);
        if (result == -1)
        {
            return -1;
        }
    }

    istack_t *isk3 = stack_rpush(program->repository, OPCODE_LEV);
    if (isk3 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    body1->n_variables = program->n_variables - body1->n_initiator;

    istack_t *isk4 = stack_insert(program->repository, isk2->next, body1->n_initiator);
    if (isk4 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk4 = stack_insert(program->repository, isk2->next, body1->n_variables);
    if (isk4 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }


    istack_t *isk5 = stack_rpop(program->repository_nvars);
    if (isk5 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tThe stack is empty\n", __FILE__, __LINE__);
        return -1;
    }
    program->n_variables = isk5->value;

    return 1;
}

static int32_t
intermediate_fun(program_t *program, node_t *node, uint64_t flag)
{
	node_fun_t *fun1 = (node_fun_t *)node->value;

    fun1->n_initiator = program->n_variables - 1;

    istack_t *isk1 = stack_rpush(program->repository_nvars, program->n_variables);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk2 = stack_rpush(program->repository, OPCODE_JMP);
    if (isk2 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk3 = stack_rpush(program->repository, OPCODE_ENT);
    if (isk3 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    fun1->entry = isk3;

    if (fun1->generics != NULL)
    {
        int32_t r1 = intermediate_generics(program, fun1->generics, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->parameters != NULL)
    {
        int32_t r1 = intermediate_parameters(program, fun1->parameters, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->body != NULL)
    {
        int32_t r1 = intermediate_body(program, fun1->body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    istack_t *isk4 = stack_rpush(program->repository, OPCODE_LEV);
    if (isk4 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk4 = stack_rpush(program->repository, OPCODE_NOP);
    if (isk4 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk5 = stack_insert(program->repository, isk2->next, (int64_t)isk4);
    if (isk5 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    fun1->n_variables = program->n_variables - fun1->n_initiator;

    istack_t *isk6 = stack_insert(program->repository, isk3->next, fun1->n_initiator);
    if (isk6 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk6 = stack_insert(program->repository, isk3->next, fun1->n_variables);
    if (isk6 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk7 = stack_rpop(program->repository_nvars);
    if (isk7 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tThe stack is empty\n", __FILE__, __LINE__);
        return -1;
    }
    program->n_variables = isk7->value;

	return 1;
}

static int32_t
intermediate_property(program_t *program, node_t *node, uint64_t flag)
{
    node_property_t *property1 = (node_property_t *)node->value;
    
    property1->n = program->n_variables;
    program->n_variables += 1;

    if (property1->value != NULL)
    {
        int32_t r1 = intermediate_expression(program, property1->value, flag);
        if (r1 == -1)
        {
            return -1;
        }

        istack_t *isk1 = stack_rpush(program->repository, OPCODE_PUSH);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_IMM);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, property1->n);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        isk1 = stack_rpush(program->repository, OPCODE_PSAVE);
        if (isk1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
    }

    return 1;
}

static int32_t
intermediate_heritage(program_t *program, node_t *node, uint64_t flag)
{
    node_heritage_t *heritage1 = (node_heritage_t *)node->value;
    
    heritage1->n = program->n_variables;
    program->n_variables += 1;

    istack_t *isk1 = stack_rpush(program->repository, OPCODE_IMM);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk1 = stack_rpush(program->repository, heritage1->n);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk1 = stack_rpush(program->repository, OPCODE_PSAVE);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    return 1;
}

static int32_t
intermediate_heritages(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *heritages = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = heritages->list->begin;a1 != heritages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = intermediate_heritage(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
intermediate_class(program_t *program, node_t *node, uint64_t flag)
{
	node_class_t *class1 = (node_class_t *)node->value;

    class1->n_initiator = program->n_variables - 1;

    istack_t *isk1 = stack_rpush(program->repository_nvars, program->n_variables);
    if (isk1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk2 = stack_rpush(program->repository, OPCODE_JMP);
    if (isk2 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk3 = stack_rpush(program->repository, OPCODE_ENT);
    if (isk3 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    class1->entry = isk3;

    if (class1->generics != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            intermediate_error(program, class1->key, "Generic type in static class\n\tInternal:%s-%u", 
                __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = intermediate_generics(program, class1->generics, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->heritages != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            intermediate_error(program, class1->key, "Generic type in inheritance\n\tInternal:%s-%u", 
                __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = intermediate_heritages(program, class1->heritages, flag);
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
                    intermediate_error(program, class2->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            int32_t r1 = intermediate_class(program, item1, flag);
            if (r1 == -1)
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
                    intermediate_error(program, fun1->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            int32_t r1 = intermediate_fun(program, item1, flag);
            if (r1 == -1)
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
                    intermediate_error(program, property1->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            int32_t r1 = intermediate_property(program, item1, flag);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }

    istack_t *isk4 = stack_rpush(program->repository, OPCODE_LEV);
    if (isk4 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk4 = stack_rpush(program->repository, OPCODE_NOP);
    if (isk4 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk5 = stack_insert(program->repository, isk2->next, (int64_t)isk4);
    if (isk5 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    class1->n_variables = program->n_variables - class1->n_initiator;

    istack_t *isk6 = stack_insert(program->repository, isk3->next, class1->n_initiator);
    if (isk6 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    isk6 = stack_insert(program->repository, isk3->next, class1->n_variables);
    if (isk6 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    istack_t *isk7 = stack_rpop(program->repository_nvars);
    if (isk7 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tThe stack is empty\n", __FILE__, __LINE__);
        return -1;
    }
    program->n_variables = isk7->value;

	return 1;
}

static int32_t
intermediate_package(program_t *program, node_t *node, uint64_t flag)
{
    node_package_t *package1 = (node_package_t *)node->value;

    if (package1->generics != NULL)
    {
        int32_t r1 = intermediate_generics(program, package1->generics, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
intermediate_packages(program_t *program, node_t *node, uint64_t flag)
{
    node_block_t *packages = (node_block_t *)node->value;
    
	ilist_t *a1;
    for (a1 = packages->list->begin;a1 != packages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t r1 = intermediate_package(program, item1, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
intermediate_using(program_t *program, node_t *node, uint64_t flag)
{
	node_using_t *using1 = (node_using_t *)node->value;

    node_basic_t *basic1 = (node_basic_t *)using1->path->value;

    node_t *node1 = program_load(program, basic1->value);
    if (node1 == NULL)
    {
        return -1;
    }

    int32_t r1 = intermediate_module(program, node1, flag);
    if (r1 == -1)
    {
        return -1;
    }

	if (using1->packages != NULL)
    {
        int32_t r1 = intermediate_packages(program, using1->packages, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }
   
	return 1;
}

int32_t
intermediate_module(program_t *program, node_t *node, uint64_t flag)
{
	node_module_t *module = (node_module_t *)node->value;

    ilist_t *a1;
    for (a1 = module->items->begin; a1 != module->items->end; a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;

        if (item1->kind == NODE_KIND_USING)
        {
            int32_t r1 = intermediate_using(program, item1, flag);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        if (item1->kind == NODE_KIND_CLASS)
        {
            int32_t r1 = intermediate_class(program, item1, flag);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }
    
	return 1;
}


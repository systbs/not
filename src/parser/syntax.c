#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include "../utils/dirent.h"
#elif defined(__arm__) || defined(__aarch64__)
#include "../utils/arm/dirent.h"
#else
#include <dirent.h>
#endif

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
#include "symbol.h"
#include "graph.h"
#include "syntax.h"

static error_t *
syntax_error(graph_t *graph, symbol_t *current, const char *format, ...)
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

	node_t *node;
	node = current->declaration;

	error_t *error;
	error = error_create(node->position, message);
	if (!error)
	{
		return NULL;
	}

	if (list_rpush(graph->errors, (list_value_t)error))
	{
		return NULL;
	}

	return error;
}



static int32_t
syntax_symbol_contain_flag(symbol_t *refrence, uint64_t flag)
{
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, flag))
		{
			return 1;
		}
	}
	return 0;
}

static symbol_t *
syntax_extract_unary(symbol_t *target, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;a != target->end;a = a->next)
	{
		if (symbol_check_flag(a, flag))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end;b = b->next)
			{
				return b;
			}
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_of_object(symbol_t *refrence, symbol_t *target);

static symbol_t *
syntax_subset_of_array(symbol_t *refrence, symbol_t *target);

static int32_t
syntax_subset_compare_string(symbol_t *refrence, char *name)
{
	node_t *node_refrence = refrence->declaration;
	node_basic_t *node_basic_refrence;
	node_basic_refrence = (node_basic_t *)node_refrence->value;

	return (strncmp(name, node_basic_refrence->value, 
		max(strlen(node_basic_refrence->value), strlen(name))) == 0);
}

static int32_t
syntax_subset_compare_id(symbol_t *refrence, symbol_t *target)
{
	node_t *node_refrence = refrence->declaration;
	node_basic_t *node_basic_refrence;
	node_basic_refrence = (node_basic_t *)node_refrence->value;

	node_t *node_target = target->declaration;
	node_basic_t *node_basic_target;
	node_basic_target = (node_basic_t *)node_target->value;

	return (strncmp(node_basic_target->value, node_basic_refrence->value, 
		max(strlen(node_basic_refrence->value), strlen(node_basic_target->value))) == 0);
}

static symbol_t *
syntax_subset_exist_in_set(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;(a != refrence->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			int32_t result;
			result = syntax_subset_compare_id(a, target);
			if(result)
			{
				return a;
			}
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_subset_of_object(a, target);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_subset_of_array(a, target);
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_of_object_property(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for(a = refrence->begin; a != refrence->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_subset_exist_in_set(a, target);
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_of_object(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for(a = refrence->begin; a != refrence->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			return syntax_subset_of_object_property(a, target);
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_of_array(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_subset_of_object(a, target);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_subset_of_array(a, target);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			int32_t result;
			result = syntax_subset_compare_id(a, target);
			if (result)
			{
				return a;
			}
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_exist_in_flag(symbol_t *refrence, symbol_t *target, uint64_t flag)
{
	symbol_t *a;
	for (a = refrence->begin;(a != refrence->end); a = a->next)
	{
		if (symbol_check_flag(a, flag))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_set(a, target);
			if(result)
			{
				return result;
			}
		}
	}
	return NULL;
}


static int32_t
syntax_duplicated(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *target)
{
	symbol_t *a;
	for(a = root->begin; a && (a != root->end); a = a->next)
	{	
		if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			} 
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			} 
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
	
		if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}

		if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
		{
			if (a == subroot)
			{
				continue;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			symbol_t *b;
			for(b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_FIELD))
				{
					symbol_t *result;
					result = syntax_subset_exist_in_flag(b, target, SYMBOL_FLAG_NAME);
					if(result && (result->id != target->id))
					{
						syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
							result->declaration->position.line, result->declaration->position.column);
						return 0;
					}
				}
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_DEFINE))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
	}

	return 1;
}



static int32_t
syntax_duplicated_method(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target);

static int32_t
syntax_duplicated_object(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current);

static int32_t
syntax_duplicated_array(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current);

static int32_t
syntax_duplicated_in_set(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_duplicated_object(graph, root, subroot, a);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_duplicated_array(graph, root, subroot, a);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			if(symbol_check_flag(subroot, SYMBOL_FLAG_METHOD))
			{
				return syntax_duplicated_method(graph, root, subroot, subroot, a);
			}
			return syntax_duplicated(graph, root, subroot, a);
		}
	}
	return 0;
}

static int32_t
syntax_duplicated_object_property(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_duplicated_in_set(graph, root, subroot, a);
		}
	}
	return 0;
}

static int32_t
syntax_duplicated_object(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			return syntax_duplicated_object_property(graph, root, subroot, a);
		}
	}
	return 0;
}

static int32_t
syntax_duplicated_array(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_duplicated_object(graph, root, subroot, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_duplicated_array(graph, root, subroot, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return syntax_duplicated(graph, root, subroot, a);
		}
	}
	return 0;
}

static int32_t
syntax_duplicated_in_name(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
		}
	}
	return result;
}



static symbol_t *
syntax_prototype_of_type(symbol_t *root, symbol_t *subroot, symbol_t *target);

static int32_t
syntax_prototype_subset_of_type_parameter(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	uint64_t refrence_counter_by_value = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			if (syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
			{
				refrence_counter_by_value += 1;
			}
			refrence_counter += 1;
		}
	}

	uint64_t target_counter = 0;
	uint64_t target_counter_by_value = 0;

	for (a = target->begin; a != target->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			if (syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
			{
				target_counter_by_value += 1;
			}
			target_counter += 1;
		}
	}

	if ((refrence_counter - refrence_counter_by_value) == (target_counter - target_counter_by_value))
	{
		return 1;
	}

	return 0;
}

static int32_t
syntax_prototype_subset_of_component_type(symbol_t *refrence, symbol_t *target)
{
	if(symbol_check_flag(refrence, SYMBOL_FLAG_OR))
	{
		int32_t result = 1;
		symbol_t *a;
		for (a = refrence->begin; a != refrence->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				result |= syntax_prototype_subset_of_component_type(a, target);
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				result |= syntax_prototype_subset_of_component_type(a, target);
			}
		}
		return result;
	}
	else if(symbol_check_flag(refrence, SYMBOL_FLAG_AND))
	{
		int32_t result = 1;
		symbol_t *a;
		for (a = refrence->begin; a != refrence->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				result &= syntax_prototype_subset_of_component_type(a, target);
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				result &= syntax_prototype_subset_of_component_type(a, target);
			}
		}
		return result;
	}
	else if (symbol_check_flag(refrence, SYMBOL_FLAG_LEFT))
	{
		if(symbol_check_flag(target, SYMBOL_FLAG_OR))
		{
			int32_t result = 1;
			symbol_t *a;
			for (a = target->begin; a != target->end; a = a->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
				{
					result |= syntax_prototype_subset_of_component_type(a, refrence);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
				{
					result |= syntax_prototype_subset_of_component_type(a, refrence);
				}
			}
			return result;
		}
		else if(symbol_check_flag(target, SYMBOL_FLAG_AND))
		{
			int32_t result = 1;
			symbol_t *a;
			for (a = target->begin; a != target->end; a = a->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
				{
					result &= syntax_prototype_subset_of_component_type(a, refrence);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
				{
					result &= syntax_prototype_subset_of_component_type(a, refrence);
				}
			}
			return result;
		}
		else
		{
			symbol_t *a;
			for (a = refrence->begin; a != refrence->end; a = a->next)
			{
				if (!symbol_equal_flag(a, target))
				{
					return 0;
				}
				if (symbol_check_flag(a, SYMBOL_FLAG_ID) || symbol_check_flag(a, SYMBOL_FLAG_NUMBER) || symbol_check_flag(a, SYMBOL_FLAG_CHAR))
				{
					if (!syntax_subset_compare_id(a, target))
					{
						return 0;
					}
					return 1;
				}
			}
		}
	}
	else if (symbol_check_flag(refrence, SYMBOL_FLAG_RIGHT))
	{
		if(symbol_check_flag(target, SYMBOL_FLAG_OR))
		{
			int32_t result = 1;
			symbol_t *a;
			for (a = target->begin; a != target->end; a = a->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
				{
					result |= syntax_prototype_subset_of_component_type(a, refrence);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
				{
					result |= syntax_prototype_subset_of_component_type(a, refrence);
				}
			}
			return result;
		}
		else if(symbol_check_flag(target, SYMBOL_FLAG_AND))
		{
			int32_t result = 1;
			symbol_t *a;
			for (a = target->begin; a != target->end; a = a->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
				{
					result &= syntax_prototype_subset_of_component_type(a, refrence);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
				{
					result &= syntax_prototype_subset_of_component_type(a, refrence);
				}
			}
			return result;
		}
		else
		{
			symbol_t *a;
			for (a = refrence->begin; a != refrence->end; a = a->next)
			{
				if (!symbol_equal_flag(a, target))
				{
					return 0;
				}
				if (symbol_check_flag(a, SYMBOL_FLAG_ID) || symbol_check_flag(a, SYMBOL_FLAG_NUMBER) || symbol_check_flag(a, SYMBOL_FLAG_CHAR))
				{
					if (!syntax_subset_compare_id(a, target))
					{
						return 0;
					}
					return 1;
				}
			}
		}
	}

	if (!symbol_equal_flag(refrence, target))
	{
		return 0;
	}

	if (symbol_check_flag(refrence, SYMBOL_FLAG_ID) || symbol_check_flag(refrence, SYMBOL_FLAG_NUMBER) || symbol_check_flag(refrence, SYMBOL_FLAG_CHAR))
	{
		if (!syntax_subset_compare_id(refrence, target))
		{
			return 0;
		}
		return 1;
	}

	return 0;
}

static int32_t
syntax_prototype_subset_of_type(symbol_t *refrence, symbol_t *target)
{
	if (!symbol_equal_flag(refrence, target))
	{
		if (!syntax_prototype_subset_of_component_type(refrence, target))
		{
			return 0;
		}
		return 1;
	}

	if (symbol_check_flag(refrence, SYMBOL_FLAG_ID) || symbol_check_flag(refrence, SYMBOL_FLAG_NUMBER) || symbol_check_flag(refrence, SYMBOL_FLAG_CHAR))
	{
		if (!syntax_subset_compare_id(refrence, target))
		{
			return 0;
		}
		return 1;
	}
	
	uint64_t refrence_counter = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		refrence_counter += 1;

		symbol_t *c;
		c = syntax_prototype_of_type(refrence, a, a);
		if(!c)
		{
			c = a;
		}

		int32_t founded = 0;
		uint64_t target_counter = 0;
		symbol_t *b;
		for (b = target->begin;b != target->end;b = b->next)
		{
			target_counter += 1;

			if (target_counter < refrence_counter)
			{
				continue;
			}

			founded = 1;

			symbol_t *d;
			d = syntax_prototype_of_type(target, b, b);
			if(!d)
			{
				d = b;
			}
			
			if (!symbol_equal_flag(c, d))
			{
				if (!syntax_prototype_subset_of_component_type(c, d))
				{
					return 0;
				}
				return 1;
			}

			if (symbol_check_flag(c, SYMBOL_FLAG_ID))
			{
				if (!syntax_subset_compare_id(c, d))
				{
					return 0;
				}
			}

			if (!syntax_prototype_subset_of_type(c, d))
			{
				return 0;
			}

			break;
		}

		if (!founded)
		{
			return 0;
		}
	}

	return 1;
}

static symbol_t *
syntax_prototype_of_type_in_type_parameter(symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end; b = b->next)
			{
				if(syntax_prototype_subset_of_type(b, target))
				{
					symbol_t *c;
					for (c = refrence->begin;c != refrence->end; c = c->next)
					{
						if (symbol_check_flag(c, SYMBOL_FLAG_TYPE))
						{
							symbol_t *d;
							for (d = c->begin;d != c->end;d = d->next)
							{
								symbol_t *e;
								e = syntax_prototype_of_type(root, a, d);
								if (e)
								{
									return e;
								}
								return d;
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}

static symbol_t *
syntax_prototype_of_type_in_heritage(symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end; b = b->next)
			{
				if(syntax_prototype_subset_of_type(b, target))
				{
					symbol_t *c;
					for (c = refrence->begin;c != refrence->end;c = c->next)
					{
						if (symbol_check_flag(c, SYMBOL_FLAG_TYPE))
						{
							symbol_t *d;
							for (d = c->begin;d != c->end;d = d->next)
							{
								symbol_t *e;
								e = syntax_prototype_of_type(root, a, d);
								if (e)
								{
									return e;
								}
								return d;
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}

static symbol_t *
syntax_prototype_of_type_in_parameter(symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end; b = b->next)
			{
				if(syntax_prototype_subset_of_type(b, target))
				{
					symbol_t *c;
					for (c = refrence->begin;c != refrence->end; c = c->next)
					{
						if (symbol_check_flag(c, SYMBOL_FLAG_TYPE))
						{
							symbol_t *d;
							for (d = c->begin;d != c->end;d = d->next)
							{
								symbol_t *e;
								e = syntax_prototype_of_type(root, a, d);
								if (e)
								{
									return e;
								}
								return d;
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}

static symbol_t *
syntax_prototype_of_type(symbol_t *root, symbol_t *subset, symbol_t *target)
{
	if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for (a = root->begin;(a != subset) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				symbol_t *b;
				b = syntax_prototype_of_type_in_type_parameter(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}

			if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				symbol_t *b = NULL;
				b = syntax_prototype_of_type_in_heritage(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
		}

		if(root->parent)
		{
			return syntax_prototype_of_type(root->parent, root, target);
		}
	}
	
	if (symbol_check_flag(root, SYMBOL_FLAG_METHOD))
	{
		symbol_t *a;
		for (a = root->begin;(a != subset) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				symbol_t *b;
				b = syntax_prototype_of_type_in_type_parameter(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
		}

		if(root->parent)
		{
			return syntax_prototype_of_type(root->parent, root, target);
		}
	}
	
	if (symbol_check_flag(root, SYMBOL_FLAG_FUNCTION))
	{
		symbol_t *a;
		for (a = root->begin;(a != subset) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				symbol_t *b;
				b = syntax_prototype_of_type_in_type_parameter(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
		}

		if(root->parent)
		{
			return syntax_prototype_of_type(root->parent, root, target);
		}
	}
	
	if (symbol_check_flag(root, SYMBOL_FLAG_TYPE))
	{
		symbol_t *a;
		for (a = root->begin;(a != subset) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				symbol_t *b;
				b = syntax_prototype_of_type_in_type_parameter(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				symbol_t *b = NULL;
				b = syntax_prototype_of_type_in_heritage(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
		}
		if(root->parent)
		{
			return syntax_prototype_of_type(root->parent, root, target);
		}
	}

	if (root->parent)
	{
		return syntax_prototype_of_type(root->parent, root, target);
	}

	return NULL;
}




static int32_t
syntax_prototype_of_parameter_type(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
		{
			refrence_counter += 1;
			int32_t founded = 0;
			uint64_t target_counter = 0;
			symbol_t *b;
			for (b = target->begin; b != target->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE))
				{
					target_counter += 1;
					if(target_counter < refrence_counter)
					{
						continue;
					}
					founded = 1;
					if(!syntax_prototype_subset_of_type(a, b))
					{
						return 0;
					}
					break;
				}
			}
			if(!founded)
			{
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
syntax_prototype_parameter_is_ellipsis(symbol_t *refrence)
{
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_ELLIPSIS))
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

static int32_t
syntax_prototype_of_array(symbol_t *refrence, symbol_t *target)
{
	if (symbol_check_flag(target, SYMBOL_FLAG_ITEM))
	{
		symbol_t *a;
		for (a = target->begin; a != target->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				if(syntax_prototype_subset_of_type(refrence, a))
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

static int32_t
syntax_prototype_of_ellipsis(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	a = syntax_extract_unary(refrence, SYMBOL_FLAG_TYPE);
	if (!a)
	{
		return 0;
	}

	symbol_t *b;
	b = syntax_extract_unary(target, SYMBOL_FLAG_TYPE);
	if (!b)
	{
		return 0;
	}

	return syntax_prototype_of_array(a, b);
}

static int32_t
syntax_prototype_of_parameter(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_parameter_counter = 0;  
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER))
		{
			if (syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
			{
				continue;
			}

			refrence_parameter_counter += 1;
			int32_t target_founded = 0;
			uint64_t target_parameter_counter = 0;
			symbol_t *b;
			for (b = target->begin; b != target->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_PARAMETER))
				{
					target_parameter_counter += 1;
					if (target_parameter_counter < refrence_parameter_counter)
					{
						continue;
					}
					target_founded = 1;

					if (syntax_prototype_parameter_is_ellipsis(a))
					{
						if (!syntax_prototype_of_ellipsis(b, a))
						{
							goto not_subset;
						}
						goto subset;
					}

					if (syntax_prototype_parameter_is_ellipsis(b))
					{
						if (!syntax_prototype_of_ellipsis(a, b))
						{
							goto not_subset;
						}
						goto subset;
					}

					if (!syntax_prototype_of_parameter_type(a, b))
					{
						goto not_subset;
					}
					break;
				}
			}
			
			if (!target_founded)
			{
				if (syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
				{
					continue;
				}
				goto not_subset;
			}
		}
	}

	uint64_t target_parameter_counter = 0;
	for (a = target->begin; a != target->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER))
		{
			target_parameter_counter += 1;
			if (target_parameter_counter <= refrence_parameter_counter)
			{
				continue;
			}

			if (syntax_prototype_parameter_is_ellipsis(a))
			{
				goto subset;
			}
			
			if (!syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
			{
				goto not_subset;
			}
		}
	}

	subset:
	return 1;

	not_subset:
	return 0;
}

static int32_t
syntax_method_exist_in_set(graph_t *graph, symbol_t *current, symbol_t *refrence, symbol_t *target_refrence, symbol_t *target)
{
	int32_t result = 1;
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			if(a->id == target->id)
			{
				break;
			}
			if(syntax_subset_compare_id(a, target))
			{
				result = syntax_prototype_subset_of_type_parameter(refrence, target_refrence);
				if(result)
				{
					result = syntax_prototype_of_parameter(refrence, target_refrence);
					if(result)
					{
						goto error;
					}
					result = syntax_prototype_of_parameter(target_refrence, refrence);
					if(result)
					{
						goto error;
					}
				}
				return 1;

				error:
				syntax_error(graph, a, "duplicated symbol(%d) in %lld:%lld\n", a->flags, 
					target->declaration->position.line, target->declaration->position.column);
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
syntax_method_exist_in_flag(graph_t *graph, symbol_t *current, symbol_t *refrence, symbol_t *target_refrence, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_method_exist_in_set(graph, a, refrence, target_refrence, target);
		}
	}
	return 1;
}

static int32_t
syntax_duplicated_method(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = root->begin; a && (a != subroot) && (a != root->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			result = syntax_method_exist_in_flag(graph, a, refrence, a, target);
			if(!result)
			{
				return 0;
			}
		}
	}
	return 1;
}


static symbol_t *
syntax_already_defined(symbol_t *root, symbol_t *subroot, symbol_t *target)
{
	symbol_t *result;
	result = syntax_subset_exist_in_flag(root, target, SYMBOL_FLAG_NAME);
	if (result)
	{
		return result;
	}

	symbol_t *a;
	for (a = root->begin; (a != subroot) && (a != root->end);a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_DEFINE))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_IF))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FOR))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FORIN))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result)
			{
				return result;
			}
		}
	}
	
	if(root->parent)
	{
		result = syntax_already_defined(root->parent, root, target);
		if(result)
		{
			return result;
		}
	}

	return NULL;
}


static symbol_t *
syntax_type_of(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *target, uint64_t flag);

static symbol_t *
syntax_equivalent_of(graph_t *graph, symbol_t *left, symbol_t *right, symbol_t *target);

static symbol_t *
syntax_equivalent_of_object(graph_t *graph, symbol_t *left, symbol_t *right, symbol_t *target);

static symbol_t *
syntax_equivalent_of_array(graph_t *graph, symbol_t *left, symbol_t *right, symbol_t *target);

static symbol_t *
syntax_object_property_extract_value_by_name(graph_t *graph, symbol_t *refrence, symbol_t *target)
{
	symbol_t *name;
	name = syntax_extract_unary(refrence, SYMBOL_FLAG_NAME);

	if (symbol_check_flag(name, SYMBOL_FLAG_ID))
	{
		if (syntax_subset_compare_id(name, target))
		{
			symbol_t *value;
			value = syntax_extract_unary(refrence, SYMBOL_FLAG_VALUE);
			if (!value)
			{
				syntax_error(graph, name, "object property without a value\n");
				return NULL;
			}
			return value;
		}
	}

	return NULL;
}

static symbol_t *
syntax_object_extract_value_by_name(graph_t *graph, symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end;a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			symbol_t *result;
			result = syntax_object_property_extract_value_by_name(graph, a, target);
			if (!result)
			{
				return result;
			}
		}
	}
	return NULL;
}

static symbol_t *
syntax_equivalent_of_object_property(graph_t *graph, symbol_t *left, symbol_t *right, symbol_t *target)
{
	symbol_t *name_left;
	name_left = syntax_extract_unary(left, SYMBOL_FLAG_NAME);

	if (symbol_check_flag(name_left, SYMBOL_FLAG_OBJECT))
	{
		symbol_t *value_left;
		value_left = syntax_extract_unary(left, SYMBOL_FLAG_VALUE);
		if (!value_left)
		{
			syntax_error(graph, name_left, "object property without a name\n");
			return NULL;
		}

		if (!symbol_check_flag(value_left, SYMBOL_FLAG_ID))
		{
			syntax_error(graph, value_left, "object property value must be a name\n");
			return NULL;
		}

		if (syntax_subset_compare_id(value_left, target))
		{
			return NULL;
		}

		symbol_t *value_right;
		value_right = syntax_object_extract_value_by_name(graph, right, value_left);
		if (!value_right)
		{
			syntax_error(graph, name_left, "no any name in right object equivalnet by left object property name\n");
			return NULL;
		}

		return syntax_equivalent_of_object(graph, name_left, value_right, target);
	}

	if (symbol_check_flag(name_left, SYMBOL_FLAG_ID))
	{
		if (syntax_subset_compare_id(name_left, target))
		{
			symbol_t *value_right;
			value_right = syntax_object_extract_value_by_name(graph, right, name_left);
			if (!value_right)
			{
				syntax_error(graph, name_left, "no any name in right object equivalnet by left object property name\n");
				return NULL;
			}
			return value_right;
		}
	}

	return NULL;
}

static symbol_t *
syntax_equivalent_of_object(graph_t *graph, symbol_t *left, symbol_t *right, symbol_t *target)
{
	if (symbol_check_flag(right, SYMBOL_FLAG_OBJECT))
	{
		symbol_t *a;
		for (a = left->begin;(a != left->end);a = a->next)
		{
			if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
			{
				symbol_t *b;
				for (b = right->begin;(b != right->end);b = b->next)
				{
					if(symbol_check_flag(b, SYMBOL_FLAG_OBJECT_PROPERTY))
					{
						symbol_t *result;
						result = syntax_equivalent_of_object_property(graph, a, b, target);
						if(result)
						{
							return result;
						}
					}
				}
			}
		}
		return NULL;
	}

	if (symbol_check_flag(right, SYMBOL_FLAG_ARRAY))
	{
		syntax_error(graph, right, "rhs of in equivalent of object could not be an array\n");
		return NULL;
	}

	return right;
}

static symbol_t *
syntax_equivalent_of_array(graph_t *graph, symbol_t *left, symbol_t *right, symbol_t *target)
{
	if (symbol_check_flag(right, SYMBOL_FLAG_ARRAY))
	{
		symbol_t *a,*b;
		for (a = left->begin,b = right->begin;(a != left->end);a = a->next,b = b->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
			{
				if (b == right->end)
				{
					syntax_error(graph, a, "rhs of in equivalent of array could not be null\n");
					return NULL;
				}
				return syntax_equivalent_of(graph, a, b, target);
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
			{
				if (b == right->end)
				{
					syntax_error(graph, a, "rhs of in equivalent of array could not be null\n");
					return NULL;
				}
				return syntax_equivalent_of(graph, a, b, target);
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_ID))
			{
				if (syntax_subset_compare_id(a, target))
				{
					return b;
				}
			}
			else
			{
				syntax_error(graph, a, "array element name\n");
				return NULL;
			}
		}
		return NULL;
	}

	if (symbol_check_flag(right, SYMBOL_FLAG_OBJECT))
	{
		syntax_error(graph, right, "rhs of in equivalent of array could not be an object\n");
		return NULL;
	}

	return right;
}

static symbol_t *
syntax_equivalent_of(graph_t *graph, symbol_t *left, symbol_t *right, symbol_t *target)
{
	if (symbol_check_flag(left, SYMBOL_FLAG_OBJECT))
	{
		return syntax_equivalent_of_object(graph, left, right, target);
	}
	if (symbol_check_flag(left, SYMBOL_FLAG_ARRAY))
	{
		return syntax_equivalent_of_array(graph, left, right, target);
	}
	if (symbol_check_flag(left, SYMBOL_FLAG_ID))
	{
		if (syntax_subset_compare_id(left, target))
		{
			return right;
		}
	}
	return NULL;
}

static int32_t
syntax_equivalent_type_parameter_by_argument(symbol_t *refrence, symbol_t *target)
{
	symbol_t *type;
	type = syntax_extract_unary(refrence, SYMBOL_FLAG_TYPE);
	if (type)
	{
		symbol_t *a;
		for (a = target->begin;a != target->end;a = a->next)
		{
			if (syntax_prototype_subset_of_type(a, type))
			{
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

static int32_t
syntax_type_match_composite(symbol_t *refrence, symbol_t *target)
{
	uint64_t ref_counter = 0;
	uint64_t tar_counter = 0;
	uint64_t tar_counter_num = 0;
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end;a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			int32_t found = 0;
			ref_counter += 1;
			symbol_t *b;
			for (b = target->begin;b != target->end;b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_ARGUMENT))
				{
					tar_counter += 1;
					if (ref_counter > tar_counter)
					{
						continue;
					}
					found = 1;
					if (!syntax_equivalent_type_parameter_by_argument(a, b))
					{
							symbol_t *value;
							value = syntax_extract_unary(a, SYMBOL_FLAG_VALUE);
							if (value)
							{
								tar_counter_num += 1;
								break;
							}
						return 0;
					}
					tar_counter_num += 1;
					break;
				}
			}
			if (tar_counter_num > ref_counter)
			{
				return 0;
			}
			if (!found)
			{
				symbol_t *value;
				value = syntax_extract_unary(a, SYMBOL_FLAG_VALUE);
				if (!value)
				{
					return 0;
				}
			}
		}
	}
	return 1;
}

static int32_t
syntax_method_is_init(symbol_t *refrence)
{
	symbol_t *name;
	name = syntax_extract_unary(refrence, SYMBOL_FLAG_NAME);
	return syntax_subset_compare_string(name, "init");
}

static int32_t
syntax_prototype_of_ellipsis_by_type(symbol_t *refrence, symbol_t *target)
{
	symbol_t *type;
	type = syntax_extract_unary(target, SYMBOL_FLAG_TYPE);
	if (!type)
	{
		return 0;
	}

	return syntax_prototype_of_array(refrence, type);
}

static int32_t
syntax_arguments_typeof_parameters(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target)
{
	uint64_t ref_counter = 0;
	uint64_t tar_counter = 0;
	uint64_t tar_counter_num = 0;
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end;a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_ARGUMENT))
		{
			int32_t found = 0;
			ref_counter += 1;

			symbol_t *b;
			for (b = target->begin;b != target->end;b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_PARAMETER))
				{
					tar_counter += 1;
					if (ref_counter > tar_counter)
					{
						continue;
					}
					found = 1;
					
					symbol_t *c;
					for (c = a->begin;c != a->end;c = c->next)
					{
						symbol_t *type;
						type = syntax_type_of(graph, root, subroot, c, SYNTAX_TYPE_OF_NONE);
						if (!type)
						{
							return 0;
						}
						
						if (syntax_prototype_parameter_is_ellipsis(b))
						{
							if (!syntax_prototype_of_ellipsis(type, b))
							{
								return 0;
							}
							return 1;
						}

						if (!syntax_prototype_of_parameter_type(type, b))
						{
							return 0;
						}

					}
					
					tar_counter_num += 1;
					break;
				}
			}
			if (tar_counter_num > ref_counter)
			{
				return 0;
			}
			if (!found)
			{
				symbol_t *value;
				value = syntax_extract_unary(a, SYMBOL_FLAG_VALUE);
				if (!value)
				{
					return 0;
				}
			}
		}
	}
	return 1;
}

static symbol_t *
syntax_type_of_attribute(symbol_t *refrence, symbol_t *target)
{
	if (symbol_check_flag(refrence, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for (a = refrence->begin;a != refrence->end;a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
			{
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);

				if (syntax_subset_compare_id(name, target))
				{
					return a;
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
			{
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);

				if (syntax_subset_compare_id(name, target))
				{
					return a;
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
			{
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);

				if (syntax_subset_compare_id(name, target))
				{
					return a;
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
			{
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);

				if (syntax_subset_compare_id(name, target))
				{
					return a;
				}
			}
		}
	}

	if (symbol_check_flag(refrence, SYMBOL_FLAG_ENUM))
	{
		symbol_t *a;
		for (a = refrence->begin;a != refrence->end;a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_ENUM_MEMBER))
			{
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);

				if (syntax_subset_compare_id(name, target))
				{
					return a;
				}
			}
		}
	}

	if (symbol_check_flag(refrence, SYMBOL_FLAG_TYPE))
	{
		symbol_t *value;
		value = syntax_extract_unary(refrence, SYMBOL_FLAG_VALUE);
		if (symbol_check_flag(value, SYMBOL_FLAG_OBJECT))
		{
			symbol_t *a;
			for (a = value->begin;a != value->end;a = a->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
				{
					symbol_t *name;
					name = syntax_extract_unary(refrence, SYMBOL_FLAG_NAME);
					if (syntax_subset_compare_id(name, target))
					{
						return a;
					}
				}
			}
		}
	}

	if (symbol_check_flag(refrence, SYMBOL_FLAG_OBJECT))
	{
		symbol_t *a;
		for (a = refrence->begin;a != refrence->end;a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
			{
				symbol_t *name;
				name = syntax_extract_unary(refrence, SYMBOL_FLAG_NAME);
				if (syntax_subset_compare_id(name, target))
				{
					return a;
				}
			}
		}
	}

	return NULL;
}

static symbol_t *
syntax_type_of(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *target, uint64_t flag)
{
	if (symbol_check_flag(target, SYMBOL_FLAG_COMPOSITE))
	{
		symbol_t *a;
		for (a = target->begin;a != target->end;a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				symbol_t *b;
				for (b = a->begin;b != a->end;b = b->next)
				{
					symbol_t *response;
					response = syntax_type_of(graph, root, subroot, b, SYNTAX_TYPE_OF_REFRENCE);
					if (response)
					{
						if (symbol_check_flag(response, SYMBOL_FLAG_CLASS))
						{
							if (!syntax_type_match_composite(response, target))
							{
								return NULL;
							}
						}
						if (symbol_check_flag(response, SYMBOL_FLAG_METHOD))
						{
							if (!syntax_type_match_composite(response, target))
							{
								return NULL;
							}
						}
						if (symbol_check_flag(response, SYMBOL_FLAG_TYPE))
						{
							if (!syntax_type_match_composite(response, target))
							{
								return NULL;
							}
						}
						if (symbol_check_flag(response, SYMBOL_FLAG_FUNCTION))
						{
							if (!syntax_type_match_composite(response, target))
							{
								return NULL;
							}
						}
						
						if (flag == SYNTAX_TYPE_OF_REFRENCE)
						{
							return response;
						}

						symbol_t *name;
						name = syntax_extract_unary(response, SYMBOL_FLAG_NAME);
						return name;
					}
					syntax_error(graph, b, "refrence of composite not found\n");
					return NULL;
				}
			}
		}
		return NULL;
	}

	if (symbol_check_flag(target, SYMBOL_FLAG_ATTR))
	{
		symbol_t *left;
		left = syntax_extract_unary(target, SYMBOL_FLAG_LEFT);
		if (left)
		{
			symbol_t *response;
			response = syntax_type_of(graph, root, subroot, left, SYNTAX_TYPE_OF_REFRENCE);
			if (response)
			{
				symbol_t *right;
				right = syntax_extract_unary(target, SYMBOL_FLAG_RIGHT);
				if (right)
				{
					symbol_t *result;
					result = syntax_type_of_attribute(response, right);
					if (result)
					{
						if (flag == SYNTAX_TYPE_OF_REFRENCE)
						{
							return result;
						}
						symbol_t *name;
						name = syntax_extract_unary(result, SYMBOL_FLAG_NAME);
						if (name)
						{
							return name;
						}
						return result;
					}
				}
			}
			syntax_error(graph, left, "refrence not found\n");
			return NULL;
		}
		return NULL;
	}
	
	if (symbol_check_flag(target, SYMBOL_FLAG_CALL))
	{
		symbol_t *a;
		for (a = target->begin;a != target->end;a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				symbol_t *b;
				for (b = a->begin;b != a->end;b = b->next)
				{
					symbol_t *response;
					response = syntax_type_of(graph, root, subroot, b, SYNTAX_TYPE_OF_REFRENCE);
					if (response)
					{
						if (symbol_check_flag(response, SYMBOL_FLAG_CLASS))
						{
							symbol_t *c;
							for (c = response->begin;c != response->end;c = c->next)
							{
								if (symbol_check_flag(c, SYMBOL_FLAG_METHOD))
								{
									if (syntax_method_is_init(c))
									{
										if (syntax_arguments_typeof_parameters(graph, root, subroot, target, c))
										{
											// delete parameters and return after
											symbol_t *name;
											name = syntax_extract_unary(response, SYMBOL_FLAG_NAME);
											return name;
										}
									}
								}
							}
							// delete parameters
						}
						if (symbol_check_flag(response, SYMBOL_FLAG_METHOD))
						{
							if (syntax_arguments_typeof_parameters(graph, root, subroot, target, response))
							{
								// delete parameters and return after
								if (flag == SYNTAX_TYPE_OF_REFRENCE)
								{
									return response;
								}
								symbol_t *name;
								name = syntax_extract_unary(response, SYMBOL_FLAG_NAME);
								return name;
							}
							// delete parameters
						}
						if (symbol_check_flag(response, SYMBOL_FLAG_FUNCTION))
						{
							if (syntax_arguments_typeof_parameters(graph, root, subroot, target, response))
							{
								// delete parameters and return after
								if (flag == SYNTAX_TYPE_OF_REFRENCE)
								{
									return response;
								}
								symbol_t *name;
								name = syntax_extract_unary(response, SYMBOL_FLAG_NAME);
								return name;
							}
							// delete parameters
						}
						syntax_error(graph, response, "missing match of arguments by (%lld:%lld)\n", 
							target->declaration->position.line, target->declaration->position.column);
						return NULL;
					}
					syntax_error(graph, b, "callable not defined\n");
					return NULL;
				}
			}
		}
		return NULL;
	}

	if (!symbol_check_flag(target, SYMBOL_FLAG_ID))
	{
		syntax_error(graph, target, "target not an id %lld\n", target->flags);
		return NULL;
	}

	symbol_t *a;
	for (a = root->begin;(a != root->end);a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				symbol_t *left;
				left = syntax_extract_unary(a, SYMBOL_FLAG_NAME);

				symbol_t *right;
				right = syntax_extract_unary(a, SYMBOL_FLAG_VALUE);

				if(!right)
				{
					symbol_t *type;
					type = syntax_extract_unary(a, SYMBOL_FLAG_TYPE);
					if (!type)
					{
						syntax_error(graph, a, "missing type of property");
						return NULL;
					}
					return type;
				}

				symbol_t *result;
				result = syntax_equivalent_of(graph, left, right, target);
				if (result && (result->id != target->id))
				{
					return syntax_type_of(graph, root, subroot, result, flag);
				}
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				if (flag == SYNTAX_TYPE_OF_REFRENCE)
				{
					return a;
				}
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);
				return name;
			} 
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				if (flag == SYNTAX_TYPE_OF_REFRENCE)
				{
					return a;
				}
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);
				return name;
			} 
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				if (flag == SYNTAX_TYPE_OF_REFRENCE)
				{
					return a;
				}
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);
				return name;
			} 
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				if (flag == SYNTAX_TYPE_OF_REFRENCE)
				{
					return a;
				}
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);
				return name;
			} 
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				symbol_t *result2;
				result2 = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_HERITAGE);
				if(result2)
				{
					continue;
				}
				if (flag == SYNTAX_TYPE_OF_REFRENCE)
				{
					return a;
				}
				symbol_t *name;
				name = syntax_extract_unary(a, SYMBOL_FLAG_NAME);
				return name;
			}
		}

		if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
		{
			if (a == subroot)
			{
				continue;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_ASSIGN))
		{
			symbol_t *left;
			left = syntax_extract_unary(a, SYMBOL_FLAG_LEFT);

			symbol_t *right;
			right = syntax_extract_unary(a, SYMBOL_FLAG_RIGHT);

			symbol_t *result;
			result = syntax_equivalent_of(graph, left, right, target);
			if (result && (result->id != target->id))
			{
				return syntax_type_of(graph, root, subroot, result, flag);
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_DEFINE))
		{
			symbol_t *left;
			left = syntax_extract_unary(a, SYMBOL_FLAG_NAME);

			symbol_t *right;
			right = syntax_extract_unary(a, SYMBOL_FLAG_VALUE);

			symbol_t *result;
			result = syntax_equivalent_of(graph, left, right, target);
			if (result && (result->id != target->id))
			{
				return syntax_type_of(graph, root, subroot, result, flag);
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			symbol_t *left;
			left = syntax_extract_unary(a, SYMBOL_FLAG_NAME);

			symbol_t *right;
			right = syntax_extract_unary(a, SYMBOL_FLAG_VALUE);

			if(!right)
			{
				syntax_error(graph, a, "missing value of const");
				return NULL;
			}
			else 
			{
				symbol_t *type;
				type = syntax_extract_unary(a, SYMBOL_FLAG_TYPE);
				if (type)
				{
					return type;
				}
			}

			symbol_t *result;
			result = syntax_equivalent_of(graph, left, right, target);
			if (result && (result->id != target->id))
			{
				return syntax_type_of(graph, root, subroot, result, flag);
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			symbol_t *left;
			left = syntax_extract_unary(a, SYMBOL_FLAG_NAME);

			symbol_t *right;
			right = syntax_extract_unary(a, SYMBOL_FLAG_VALUE);

			if(!right)
			{
				symbol_t *type;
				type = syntax_extract_unary(a, SYMBOL_FLAG_TYPE);
				if (!type)
				{
					syntax_error(graph, a, "missing type of var");
					return NULL;
				}
				return type;
			}

			symbol_t *result;
			result = syntax_equivalent_of(graph, left, right, target);
			if (result && (result->id != target->id))
			{
				return syntax_type_of(graph, root, subroot, result, flag);
			}
		}

	}

	if (root->parent)
	{
		return syntax_type_of(graph, root->parent, root, target, flag);
	}

	return NULL;
}


static int32_t
syntax_analysis_expression(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static int32_t
syntax_analysis_postfix(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static int32_t
syntax_analysis_bitwise_or(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static int32_t
syntax_analysis_block(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static int32_t
syntax_analysis_function(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static int32_t
syntax_analysis_id(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t * result;
	result = syntax_already_defined(root, subroot, current);
	if (!result)
	{
		syntax_error(graph, current, "symbol(%d) already not defined\n", current->flags);
		return 0;
	}
	return 1;
}

static int32_t
syntax_analysis_parenthesis(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		result &= syntax_analysis_expression(graph, current, a, a);
		if(!result)
		{
			return 0;
		}
	}
	return result;
}

static int32_t
syntax_analysis_array(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		result &= syntax_analysis_expression(graph, current, a, a);
		if(!result)
		{
			return 0;
		}
	}
	return result;
}

static int32_t
syntax_analysis_object_property(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_expression(graph, a, b, b);
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VALUE))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_expression(graph, a, b, b);
			}
		}
	}
	return result;
}

static int32_t
syntax_analysis_object(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_object_property(graph, a, b, b);
			}
		}
	}
	return result;
}

static int32_t
syntax_analysis_primary(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (symbol_check_flag(current, SYMBOL_FLAG_ID))
	{
		result &= syntax_analysis_id(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_ARRAY))
	{
		result &= syntax_analysis_array(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_OBJECT))
	{
		result &= syntax_analysis_object(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_PARENTHESIS))
	{
		result &= syntax_analysis_parenthesis(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FUNCTION))
	{
		result &= syntax_analysis_function(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_composite(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_postfix(graph, a, b, b);
			}
		}
		if (symbol_check_flag(a, SYMBOL_FLAG_ARGUMENT))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_expression(graph, a, b, b);
			}
		}
	}
	return result;
}

static int32_t
syntax_analysis_call(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_postfix(graph, a, b, b);
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARGUMENT))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_expression(graph, a, b, b);
			}
		}
	}
	return result;
}

static int32_t
syntax_analysis_get_slice(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_postfix(graph, a, b, b);
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_GET_START))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_expression(graph, a, b, b);
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_GET_STEP))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_expression(graph, a, b, b);
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_GET_STOP))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_expression(graph, a, b, b);
			}
		}
	}
	return result;
}

static int32_t
syntax_analysis_get_item(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_postfix(graph, a, b, b);
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_INDEX))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_expression(graph, a, b, b);
			}
		}
	}
	return result;
}

static int32_t
syntax_analysis_get_attr(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *b;
	b = syntax_type_of(graph, root, subroot, current, SYNTAX_TYPE_OF_NONE);
	if (!b)
	{
		return 0;
	}

	int32_t result = 1;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_postfix(graph, a, b, b);
			}
		}
		if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				result &= syntax_analysis_id(graph, a, b, b);
			}
		}
	}
	return result;
}

static int32_t
syntax_analysis_postfix(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (symbol_check_flag(current, SYMBOL_FLAG_COMPOSITE))
	{
		result &= syntax_analysis_composite(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_CALL))
	{
		result &= syntax_analysis_call(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_SLICE))
	{
		result &= syntax_analysis_get_slice(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_ITEM))
	{
		result &= syntax_analysis_get_item(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_ATTR))
	{
		result &= syntax_analysis_get_attr(graph, root, subroot, current);
	}
	else
	{
		result &= syntax_analysis_primary(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_prefix(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (
		symbol_check_flag(current, SYMBOL_FLAG_TILDE) || 
		symbol_check_flag(current, SYMBOL_FLAG_NOT) ||
		symbol_check_flag(current, SYMBOL_FLAG_NEG) ||
		symbol_check_flag(current, SYMBOL_FLAG_POS) ||
		symbol_check_flag(current, SYMBOL_FLAG_GET_VALUE) ||
		symbol_check_flag(current, SYMBOL_FLAG_GET_ADDRESS) ||
		symbol_check_flag(current, SYMBOL_FLAG_AWAIT) ||
		symbol_check_flag(current, SYMBOL_FLAG_SIZEOF) ||
		symbol_check_flag(current, SYMBOL_FLAG_TYPEOF) ||
		symbol_check_flag(current, SYMBOL_FLAG_ELLIPSIS))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *c;
				for (c = a->begin;(c != a->end); c = c->next)
				{
					if (
						symbol_check_flag(c, SYMBOL_FLAG_TILDE) || 
						symbol_check_flag(c, SYMBOL_FLAG_NOT) ||
						symbol_check_flag(c, SYMBOL_FLAG_NEG) ||
						symbol_check_flag(c, SYMBOL_FLAG_POS) ||
						symbol_check_flag(c, SYMBOL_FLAG_GET_VALUE) ||
						symbol_check_flag(c, SYMBOL_FLAG_GET_ADDRESS) ||
						symbol_check_flag(c, SYMBOL_FLAG_AWAIT) ||
						symbol_check_flag(c, SYMBOL_FLAG_SIZEOF) ||
						symbol_check_flag(c, SYMBOL_FLAG_TYPEOF) ||
						symbol_check_flag(c, SYMBOL_FLAG_ELLIPSIS))
					{
						result &= syntax_analysis_prefix(graph, a, c, c);
					}
					else
					{
						result &= syntax_analysis_postfix(graph, a, c, c);
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_postfix(graph, a, b, b);
				}
			}
		}
	}
	else
	{
		result &= syntax_analysis_postfix(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_multiplicative(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (
		symbol_check_flag(current, SYMBOL_FLAG_MUL) || 
		symbol_check_flag(current, SYMBOL_FLAG_DIV) ||
		symbol_check_flag(current, SYMBOL_FLAG_MOD))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_MUL) || 
						symbol_check_flag(b, SYMBOL_FLAG_DIV) ||
						symbol_check_flag(b, SYMBOL_FLAG_MOD))
					{
						result &= syntax_analysis_multiplicative(graph, a, b, b);
					}
					else
					{
						result &= syntax_analysis_prefix(graph, a, b, b);
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_prefix(graph, a, b, b);
				}
			}
		}
	}
	else
	{
		result &= syntax_analysis_prefix(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_addative(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (
		symbol_check_flag(current, SYMBOL_FLAG_PLUS) || 
		symbol_check_flag(current, SYMBOL_FLAG_MINUS))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_PLUS) || 
						symbol_check_flag(b, SYMBOL_FLAG_MINUS))
					{
						result &= syntax_analysis_addative(graph, a, b, b);
					}
					else
					{
						result &= syntax_analysis_multiplicative(graph, a, b, b);
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_multiplicative(graph, a, b, b);
				}
			}
		}
	}
	else
	{
		result &= syntax_analysis_multiplicative(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_shifting(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (
		symbol_check_flag(current, SYMBOL_FLAG_SHL) || 
		symbol_check_flag(current, SYMBOL_FLAG_SHR))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_SHL) || 
						symbol_check_flag(b, SYMBOL_FLAG_SHR))
					{
						result &= syntax_analysis_shifting(graph, a, b, b);
					}
					else
					{
						result &= syntax_analysis_addative(graph, a, b, b);
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_addative(graph, a, b, b);
				}
			}
		}
	} else {
		result &= syntax_analysis_addative(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_relational(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (
		symbol_check_flag(current, SYMBOL_FLAG_LT) || 
		symbol_check_flag(current, SYMBOL_FLAG_GT) || 
		symbol_check_flag(current, SYMBOL_FLAG_LE) ||
		symbol_check_flag(current, SYMBOL_FLAG_GE))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_LT) || 
						symbol_check_flag(b, SYMBOL_FLAG_GT) || 
						symbol_check_flag(b, SYMBOL_FLAG_LE) ||
						symbol_check_flag(b, SYMBOL_FLAG_GE))
					{
						result &= syntax_analysis_relational(graph, a, b, b);
					}
					else
					{
						result &= syntax_analysis_shifting(graph, a, b, b);
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_shifting(graph, a, b, b);
				}
			}
		}
	} else {
		result &= syntax_analysis_shifting(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_equality(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (
		symbol_check_flag(current, SYMBOL_FLAG_EQ) || 
		symbol_check_flag(current, SYMBOL_FLAG_IN) || 
		symbol_check_flag(current, SYMBOL_FLAG_NEQ) ||
		symbol_check_flag(current, SYMBOL_FLAG_EXTENDS))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_EQ) || 
						symbol_check_flag(b, SYMBOL_FLAG_IN) || 
						symbol_check_flag(b, SYMBOL_FLAG_NEQ))
					{
						result &= syntax_analysis_equality(graph, a, b, b);
					}
					else
					{
						result &= syntax_analysis_relational(graph, a, b, b);
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_relational(graph, a, b, b);
				}
			}
		}
	}
	else
	{
		result &= syntax_analysis_relational(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_bitwise_and(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (symbol_check_flag(current, SYMBOL_FLAG_AND))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (symbol_check_flag(b, SYMBOL_FLAG_AND))
					{
						result &= syntax_analysis_bitwise_and(graph, a, b, b);
					}
					else
					{
						result &= syntax_analysis_equality(graph, a, b, b);
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_equality(graph, a, b, b);
				}
			}
		}
	}
	else
	{
		result &= syntax_analysis_equality(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_bitwise_xor(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (symbol_check_flag(current, SYMBOL_FLAG_XOR))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (symbol_check_flag(b, SYMBOL_FLAG_XOR))
					{
						result &= syntax_analysis_bitwise_xor(graph, a, b, b);
					}
					else
					{
						result &= syntax_analysis_bitwise_and(graph, a, b, b);
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_bitwise_and(graph, a, b, b);
				}
			}	
		}
	}
	else
	{
		result &= syntax_analysis_bitwise_and(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_bitwise_or(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (symbol_check_flag(current, SYMBOL_FLAG_OR))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (symbol_check_flag(b, SYMBOL_FLAG_OR))
					{
						result &= syntax_analysis_bitwise_or(graph, a, b, b);
					}
					else
					{
						result &= syntax_analysis_bitwise_xor(graph, a, b, b);
					}
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_bitwise_xor(graph, a, b, b);
				}
			}
		}
	}	else {
		result &= syntax_analysis_bitwise_xor(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_logical_and(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (symbol_check_flag(current, SYMBOL_FLAG_LAND))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_bitwise_or(graph, a, b, b);
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_bitwise_or(graph, a, b, b);
				}
			}
		}
	}
	else
	{
		result &= syntax_analysis_bitwise_or(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_logical_or(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (symbol_check_flag(current, SYMBOL_FLAG_LOR))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_logical_and(graph, a, b, b);
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_logical_and(graph, a, b, b);
				}
			}
		}
	}
	else
	{
		result &= syntax_analysis_logical_and(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_expression(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	if (symbol_check_flag(current, SYMBOL_FLAG_CONDITIONAL))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_CONDITION))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_logical_or(graph, a, b, b);
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_TRUE))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_expression(graph, a, b, b);
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_FALSE))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_expression(graph, a, b, b);
				}
			}
		}
	}
	else
	{
		result &= syntax_analysis_logical_or(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_assign(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	
	if (symbol_check_flag(current, SYMBOL_FLAG_ASSIGN))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_expression(graph, a, b, b);
				}
			}
		
			if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					result &= syntax_analysis_expression(graph, a, b, b);
				}
			}
		}
	}
	else if (!symbol_check_flag(current, SYMBOL_FLAG_DEFINE))
	{
		result &= syntax_analysis_expression(graph, root, subroot, current);
	}
	return result;
}

static int32_t
syntax_analysis_return(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		result &= syntax_analysis_expression(graph, current, a, a);
		if(!result)
		{
			return result;
		}
	}

	return result;
}

static int32_t
syntax_analysis_continue(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		result &= syntax_analysis_expression(graph, current, a, a);
	}

	return result;
}

static int32_t
syntax_analysis_break(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		result &= syntax_analysis_expression(graph, current, a, a);
	}

	return result;
}

static int32_t
syntax_analysis_throw(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		result &= syntax_analysis_expression(graph, current, a, a);
	}

	return result;
}

static int32_t
syntax_analysis_if(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_analysis_block(graph, current, a, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ELSE))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_IF))
				{
					result &= syntax_analysis_if(graph, current, a, a);
				}
				else
				{
					result &= syntax_analysis_block(graph, current, a, a);
				}
			}
		}
	}

	return result;
}

static int32_t
syntax_analysis_catch(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_analysis_block(graph, current, a, a);
		}
	}

	return result;
}

static int32_t
syntax_analysis_try(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_CATCH))
		{
			result &= syntax_analysis_catch(graph, current, a, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_analysis_block(graph, current, a, a);
		}
	}

	return result;
}

static int32_t
syntax_analysis_for_init(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_ASSIGN))
		{
			result &= syntax_analysis_assign(graph, current, a, a);
		}
	}

	return result;
}

static int32_t
syntax_analysis_for(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_FOR_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_FOR_INIT))
				{
					result &= syntax_analysis_for_init(graph, a, b, b);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_ASSIGN))
				{
					result &= syntax_analysis_assign(graph, a, b, b);
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_analysis_block(graph, current, a, a);
		}
	}

	return result;
}

static int32_t
syntax_analysis_forin(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_FORIN_EXPRESSION))
		{
			result &= syntax_analysis_expression(graph, current, a, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_analysis_block(graph, current, a, a);
		}
	}

	return result;
}

static int32_t
syntax_analysis_function(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_analysis_block(graph, current, a, a);
		}
	}

	return result;
}

static int32_t
syntax_analysis_statement(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	if (symbol_check_flag(current, SYMBOL_FLAG_BLOCK))
	{
		result &= syntax_analysis_block(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_IF))
	{
		result &= syntax_analysis_if(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_TRY))
	{
		result &= syntax_analysis_try(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FOR))
	{
		result &= syntax_analysis_for(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FORIN))
	{
		result &= syntax_analysis_forin(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FUNCTION))
	{
		result &= syntax_analysis_function(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_CONTINUE))
	{
		result &= syntax_analysis_continue(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_BREAK))
	{
		result &= syntax_analysis_break(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_RETURN))
	{
		result &= syntax_analysis_return(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_THROW))
	{
		result &= syntax_analysis_throw(graph, root, subroot, current);
	}
	else if (
		!(symbol_check_flag(current, SYMBOL_FLAG_VAR) || 
		symbol_check_flag(current, SYMBOL_FLAG_CONST) || 
		symbol_check_flag(current, SYMBOL_FLAG_TYPE))
		)
	{
		result &= syntax_analysis_assign(graph, root, subroot, current);
	}

	return result;
}

static int32_t
syntax_analysis_block(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		result &= syntax_analysis_statement(graph, current, a, a);
	}

	return result;
}





static int32_t
syntax_block(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static int32_t
syntax_var(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_const(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_if(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_block(graph, root, subroot, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ELSE))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_IF))
				{
					result &= syntax_if(graph, root, subroot, a);
				}
				else
				{
					result &= syntax_block(graph, root, subroot, a);
				}
			}
		}
	}

	return result;
}

static int32_t
syntax_parameter(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_catch(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_block(graph, subroot, a, a);
		}
	}

	return result;
}

static int32_t
syntax_try(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_CATCH))
		{
			result &= syntax_catch(graph, subroot, a, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_block(graph, subroot, a, a);
		}
	}

	return result;
}

static int32_t
syntax_for_init(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result &= syntax_var(graph, current, a, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			result &= syntax_const(graph, current, a, a);
		}
	}

	return result;
}

static int32_t
syntax_for(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_FOR_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_FOR_INIT))
				{
					result &= syntax_for_init(graph, a, b, b);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
				{
					result &= syntax_var(graph, a, b, b);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
				{
					result &= syntax_const(graph, a, b, b);
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_block(graph, subroot, a, a);
		}
	}

	return result;
}

static int32_t
syntax_forin(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_FORIN_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
				{
					result &= syntax_var(graph, a, b, b);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
				{
					result &= syntax_const(graph, a, b, b);
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_block(graph, subroot, a, a);
		}
	}

	return result;
}

static int32_t
syntax_statement(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	if (symbol_check_flag(current, SYMBOL_FLAG_BLOCK))
	{
		result &= syntax_block(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_IF))
	{
		result &= syntax_if(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_TRY))
	{
		result &= syntax_try(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FOR))
	{
		result &= syntax_for(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FORIN))
	{
		result &= syntax_forin(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_VAR))
	{
		result &= syntax_var(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_CONST))
	{
		result &= syntax_const(graph, root, subroot, current);
	}

	return result;
}

static int32_t
syntax_block(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	
	result &= syntax_analysis_block(graph, root, subroot, current);
	if (!result)
	{
		return 0;
	}
	
	symbol_t *a;
	for (a = current->begin;(a != current->end);a = a->next)
	{
		result &= syntax_statement(graph, subroot, a, a);
	}

	return result;
}

static int32_t
syntax_import(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			result &= syntax_duplicated_in_name(graph, current, a, a);
			if(!result)
			{
				return 0;
			}
			result &= syntax_duplicated_in_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_type_parameter(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, a, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_heritage(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, a, a);
			if(!result)
			{
				return 0;
			}
			symbol_t *b;
			for(b = root->begin; b != root->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= syntax_duplicated_in_name(graph, root, b, b);
					if(!result)
					{
						return result;
					}
				}
			}
		}
	}

	return result;
}

static int32_t
syntax_method(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = root->begin; a != root->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			result &= syntax_duplicated_in_name(graph, root, a, current);
			if(!result)
			{
				return result;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			result &= syntax_duplicated_in_name(graph, root, a, current);
			if(!result)
			{
				return result;
			}
		}
	}

	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
			if (!result)
			{
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_block(graph, current, a, a);
			if (!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_enum(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for(a = root->begin; a != root->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				result &= syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return result;
				}
			}
			
			if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				result &= syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return result;
				}
			}
		}
	}

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_property(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for(a = root->begin; a != root->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				result &= syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return result;
				}
			}
			
			if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				result &= syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return result;
				}
			}
		}
	}

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_class(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for(a = root->begin; a != root->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				result &= syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return result;
				}
			}
			
			if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				result &= syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return result;
				}
			}
		}
	}

	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		// attribute
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
			if (!result)
			{
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			result &= syntax_heritage(graph, root, subroot, a);
			if (!result)
			{
				return 0;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			result &= syntax_type_parameter(graph, root, subroot, a);
			if(!result)
			{
				return 0;
			}
		}
		
		// subclass
		if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result &= syntax_class(graph, current, a, a);
			if(!result)
			{
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			result &= syntax_method(graph, current, a, a);
			if (!result)
			{
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result &= syntax_enum(graph, current, a, a);
			if(!result)
			{
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			result &= syntax_property(graph, current, a, a);
			if(!result)
			{
				return 0;
			}
		}
		
	}

	return result;
}

static int32_t
syntax_type(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_func(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
			if (!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			result &= syntax_block(graph, root, subroot, a);
			if (!result)
			{
				return 0;
			}
		}
	}

	return result;
}



static int32_t
syntax_module(graph_t *graph, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			result &= syntax_import(graph, current, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result &= syntax_class(graph, current, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result &= syntax_enum(graph, current, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result &= syntax_var(graph, current, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			result &= syntax_const(graph, current, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			result &= syntax_func(graph, current, current, a);
		}
	}

	return result;
}

int32_t
syntax_run(graph_t *graph)
{
	symbol_t *symbol;
	symbol = (symbol_t *)graph->symbol;

	int32_t result = 1;

	symbol_t *a;
	for(a = symbol->begin; a != symbol->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_MODULE))
		{
			result &= syntax_module(graph, a);
		}
	}

	return result;
}



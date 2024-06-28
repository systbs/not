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

int32_t
not_assign_set_value(not_node_t *node, not_record_t *left, not_record_t *right)
{
    if (left->readonly)
    {
        not_error_type_by_node(node, "readonly variable");
        return -1;
    }

    if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            mpz_set(*(mpz_t *)(left->value), *(mpz_t *)(right->value));

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "float");
                return -1;
            }

            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = not_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_set_si(*(mpz_t *)(left->value), *(char *)(right->value));

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "string");
                return -1;
            }

            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)) + 1);
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "object");
                return -1;
            }

            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "tuple");
                return -1;
            }

            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "type");
                return -1;
            }

            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "struct");
                return -1;
            }

            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "null");
                return -1;
            }

            mpz_clear(*(mpz_t *)(left->value));
            not_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;
                return 0;
            }

            mpz_clear(*(mpz_t *)(left->value));
            not_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                left->nan = 1;
                return 0;
            }

            mpz_clear(*(mpz_t *)(left->value));
            not_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            mpf_set_z(*(mpf_t *)left->value, *(mpz_t *)(right->value));

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_set(*(mpf_t *)left->value, *(mpf_t *)(right->value));

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpf_set_d(*(mpf_t *)left->value, *(char *)(right->value));

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "string");
                return -1;
            }

            mpf_clear(*(mpf_t *)left->value);
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)) + 1);
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "object");
                return -1;
            }

            mpf_clear(*(mpf_t *)left->value);
            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "tuple");
                return -1;
            }

            mpf_clear(*(mpf_t *)left->value);
            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "type");
                return -1;
            }

            mpf_clear(*(mpf_t *)left->value);
            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "struct");
                return -1;
            }

            mpf_clear(*(mpf_t *)left->value);
            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            left->null = 1;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;
                return 0;
            }

            mpf_clear(*(mpf_t *)left->value);
            not_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                left->nan = 1;
                return 0;
            }

            mpf_clear(*(mpf_t *)left->value);
            not_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "int");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "float");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            *(char *)left->value = *(char *)(right->value);

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "string");
                return -1;
            }

            void *ptr = not_memory_calloc(strlen((char *)(right->value)) + 1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "object");
                return -1;
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "tuple");
                return -1;
            }

            if (left->reference == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "tuple");
                return -1;
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "struct");
                return -1;
            }

            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "null");
                return -1;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;
                return 0;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "nan");
                return -1;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "int");
                return -1;
            }

            void *ptr = not_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "float");
                return -1;
            }

            void *ptr = not_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "char");
                return -1;
            }

            void *ptr = not_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)) + 1);
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }

            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "object");
                return -1;
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "tuple");
                return -1;
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "struct");
                return -1;
            }

            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "null");
                return -1;
            }

            not_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;
                return 0;
            }

            not_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "nan");
                return -1;
            }

            not_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (left->reference == 0)
            {
                not_memory_free(left->value);
            }

            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }

            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }

            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }

            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            void *ptr = not_memory_calloc(1, strlen((char *)(right->value)) + 1);
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }

            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                int32_t r1 = not_execute_value_check_by_value(node, left, right);
                if (r1 == -1)
                {
                    return -1;
                }
                else if (r1 == 0)
                {
                    not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                    return -1;
                }
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_record_object_destroy((not_record_object_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_record_tuple_destroy((not_record_tuple_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_record_type_destroy((not_record_type_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_record_struct_destroy((not_record_struct_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;
                return 0;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_record_proc_destroy((not_record_proc_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                not_record_builtin_destroy((not_record_builtin_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "int");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }

            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "float");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }

            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "char");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }

            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "string");
                return -1;
            }

            void *ptr = not_memory_calloc(1, strlen((char *)(right->value)) + 1);
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }

            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "object");
                return -1;
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_record_object_destroy((not_record_object_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                int32_t r1 = not_execute_value_check_by_value(node, left, right);
                if (r1 == -1)
                {
                    return -1;
                }
                else if (r1 == 0)
                {
                    not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "tuple");
                    return -1;
                }
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_record_tuple_destroy((not_record_tuple_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_record_type_destroy((not_record_type_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "struct");
                return -1;
            }

            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_record_struct_destroy((not_record_struct_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "undefined");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;
                return 0;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "nan");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_record_proc_destroy((not_record_proc_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "tuple", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                not_record_builtin_destroy((not_record_builtin_t *)ptr);
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "int");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "float");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "char");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "string");
                return -1;
            }

            void *ptr = not_memory_calloc(1, strlen((char *)(right->value)) + 1);
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "object");
                return -1;
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_record_object_destroy((not_record_object_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "tuple");
                return -1;
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_record_tuple_destroy((not_record_tuple_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_record_type_destroy((not_record_type_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "struct");
                return -1;
            }

            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_record_struct_destroy((not_record_struct_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "null");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;
                return 0;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "nan");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_record_proc_destroy((not_record_proc_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "type", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                not_record_builtin_destroy((not_record_builtin_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "int");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "float");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "char");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "string");
                return -1;
            }

            void *ptr = not_memory_calloc(1, strlen((char *)(right->value)) + 1);
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "object");
                return -1;
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_record_object_destroy((not_record_object_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "tuple");
                return -1;
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_record_tuple_destroy((not_record_tuple_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_record_type_destroy((not_record_type_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                int32_t r1 = not_execute_value_check_by_value(node, left, right);
                if (r1 == -1)
                {
                    return -1;
                }
                else if (r1 == 0)
                {
                    not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "struct");
                    return -1;
                }
            }

            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_record_struct_destroy((not_record_struct_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                left->null = 1;
                return 0;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;
                return 0;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "nan");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_record_proc_destroy((not_record_proc_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "struct", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                not_record_proc_destroy((not_record_proc_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "int");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "float");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "char");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "string");
                return -1;
            }

            void *ptr = not_memory_calloc(strlen((char *)(right->value)) + 1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "object");
                return -1;
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "tuple");
                return -1;
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;
                return 0;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "nan");
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));

            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));

            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));

            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            void *ptr = not_memory_calloc(strlen((char *)(right->value)) + 1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));

            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));

            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));

            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "char");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));

            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "string");
                return -1;
            }

            void *ptr = not_memory_calloc(strlen((char *)(right->value)) + 1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));

            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "object");
                return -1;
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "tuple");
                return -1;
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "struct");
                return -1;
            }

            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "null");
                return -1;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;

                return 0;
            }

            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "proc");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_PROC)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "int");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "float");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "char");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "string");
                return -1;
            }

            void *ptr = not_memory_calloc(strlen((char *)(right->value)) + 1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "object");
                return -1;
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_record_object_destroy((not_record_object_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "tuple");
                return -1;
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_record_tuple_destroy((not_record_tuple_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_record_type_destroy((not_record_type_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "struct");
                return -1;
            }

            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_record_struct_destroy((not_record_struct_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "null");
                return -1;
            }

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                return -1;
            }
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;

                return 0;
            }

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                return -1;
            }
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "nan");
                return -1;
            }

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                return -1;
            }
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_record_proc_destroy((not_record_proc_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "proc", "builtin");
                return -1;
            }

            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_proc_destroy((not_record_proc_t *)left->value) < 0)
            {
                not_record_builtin_destroy((not_record_builtin_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_BUILTIN)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "int");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "float");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "char");
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "string");
                return -1;
            }

            void *ptr = not_memory_calloc(strlen((char *)(right->value)) + 1, sizeof(char));
            if (ptr == NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_memory_free(ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "object");
                return -1;
            }

            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_record_object_destroy((not_record_object_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "tuple");
                return -1;
            }

            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_record_tuple_destroy((not_record_tuple_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "type");
                return -1;
            }

            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_record_type_destroy((not_record_type_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "struct");
                return -1;
            }

            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_record_struct_destroy((not_record_struct_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "null");
                return -1;
            }

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                return -1;
            }
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                left->undefined = 1;
                left->nan = right->nan;
                left->null = right->null;

                return 0;
            }

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                return -1;
            }
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "nan");
                return -1;
            }

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                return -1;
            }
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_PROC)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "builtin", "builtin");
                return -1;
            }

            void *ptr = not_record_proc_copy((not_record_proc_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_record_proc_destroy((not_record_proc_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_PROC;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }
        else if (right->kind == RECORD_KIND_BUILTIN)
        {
            void *ptr = not_record_builtin_copy((not_record_builtin_t *)right->value);
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }

            if (not_record_builtin_destroy((not_record_builtin_t *)left->value) < 0)
            {
                not_record_builtin_destroy((not_record_builtin_t *)ptr);
                return -1;
            }
            left->value = ptr;
            left->kind = RECORD_KIND_BUILTIN;

            left->undefined = right->undefined;
            left->nan = right->nan;
            left->null = right->null;

            return 0;
        }

        return 0;
    }

    return 0;
}

int32_t
not_assign(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_assign_set_value(node, left, right) < 0)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_ADD_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_addative_plus(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_SUB_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_addative_minus(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_MUL_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_multipicative_mul(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_DIV_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_addative_plus(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_EPI_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_multipicative_epi(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_MOD_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_multipicative_mod(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_POW_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_power_pow(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_SHL_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_shifting_shl(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_SHR_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_shifting_shr(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_AND_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_execute_and(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_OR_ASSIGN)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_expression(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return -1;
        }

        not_record_t *right = not_expression(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return -1;
        }

        not_record_t *result = not_execute_or(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            not_record_link_decrease(right);

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_assign_set_value(node, left, result) < 0)
        {
            not_record_link_decrease(left);
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return -1;
        }

        if (not_record_link_decrease(result) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        not_record_t *value = not_expression(node, strip, applicant, origin);
        if (value == NOT_PTR_ERROR)
        {
            return -1;
        }

        if (not_record_link_decrease(value) < 0)
        {
            return -1;
        }

        return 0;
    }
}

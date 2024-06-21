#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <float.h>
#include <jansson.h>

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
#include "../../memory.h"
#include "../record.h"

#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

int32_t
not_execute_set_value(not_node_t *node, not_record_t *left, not_record_t *right)
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
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = not_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

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
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = not_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_ui(*(mpz_t *)(ptr), *(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

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
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

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
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "undefined");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "nan");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = not_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_z(*(mpf_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = not_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = not_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_d(*(mpf_t *)(ptr), *(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

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
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

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

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "null");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "undefined");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "nan");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

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

            void *ptr = not_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "float");
                return -1;
            }

            void *ptr = not_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(char));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "string");
                return -1;
            }
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

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
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
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
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

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
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "null");
                return -1;
            }
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "undefined");
                return -1;
            }
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "nan");
                return -1;
            }
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

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
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

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
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(char));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

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
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "undefined");
                return -1;
            }
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

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
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

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

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
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

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (not_record_object_destroy((not_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

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

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

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
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (not_record_tuple_destroy((not_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

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
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (not_record_type_destroy((not_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

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
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, sizeof(char));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = not_memory_calloc(1, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
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

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                not_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (not_record_struct_destroy((not_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(char));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(char));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

            return 0;
        }

        return 0;
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = not_memory_realloc(left->value, sizeof(char));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            void *ptr = not_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;

            return 0;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            void *ptr = not_record_object_copy((not_record_object_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            void *ptr = not_record_tuple_copy((not_record_tuple_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;

            return 0;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            void *ptr = not_record_type_copy((not_record_type_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;

            return 0;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            void *ptr = not_record_struct_copy((not_record_struct_t *)(right->value));
            if (ptr == NOT_PTR_ERROR)
            {
                return -1;
            }
            not_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NULL;

            return 0;
        }
        else if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_UNDEFINED;

            return 0;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_memory_free(left->value);
            left->value = NOT_PTR_NULL;
            left->kind = RECORD_KIND_NAN;

            return 0;
        }

        return 0;
    }

    return 0;
}

int32_t
not_execute_assign(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_execute_set_value(node, left, right) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_addative_plus(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_addative_minus(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_multipicative_mul(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_addative_plus(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_multipicative_epi(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_multipicative_mod(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_power_pow(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_shifting_shl(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_shifting_shr(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_execute_and(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        not_record_t *result = not_execute_or(node, left, right, applicant);
        if (result == NOT_PTR_ERROR)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }

            if (not_record_link_decrease(right) < 0)
            {
                return -1;
            }

            return -1;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return -1;
        }

        if (not_execute_set_value(node, left, result) < 0)
        {
            if (not_record_link_decrease(left) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(left) < 0)
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
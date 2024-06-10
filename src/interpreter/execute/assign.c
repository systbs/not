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
#include "../../memory.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

int32_t
sy_execute_set_value(sy_node_t *node, sy_record_t *left, sy_record_t *right)
{
    if (left->readonly)
    {
        sy_error_type_by_node(node, "readonly assignment");
        return -1;
    }

    if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "float");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_ui(*(mpz_t *)(ptr), *(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "string");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "object");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "tuple");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "type");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "struct");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "null");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "undefined");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int", "nan");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_z(*(mpf_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_d(*(mpf_t *)(ptr), *(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "string");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "object");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "tuple");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "type");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "struct");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "null");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "undefined");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float", "nan");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "int");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "float");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "int");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "float");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            void *ptr = sy_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int");
                return -1;
            }
            
            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int");
                return -1;
            }
            
            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int");
                return -1;
            }
            
            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int");
                return -1;
            }
            
            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "int");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "float");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "char");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "int");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "float");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "char");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_INT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "int");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "float");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "char");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen((char *)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy((char *)(ptr), (char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }

    return 0;
}

int32_t
sy_execute_assign(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        if (sy_execute_set_value(node, left, right) < 0)
        {
            left->link -= 1;
            right->link -= 1;
            return -1;
        }
        
        left->link -= 1;
        right->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_ADD_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_plus(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;

            left->link -= 1;

            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_SUB_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_minus(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_MUL_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_mul(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_DIV_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_plus(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_EPI_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_epi(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_MOD_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_mod(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;
        return 0;
    }
    else
    if (node->kind == NODE_KIND_POW_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_pow(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_SHL_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_shl(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_SHR_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_shr(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_AND_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_and(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;

            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_OR_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_or(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;
        return 0;
    }
    else
    {
        sy_record_t *value = sy_execute_expression(node, strip, applicant, origin);
        if (value == ERROR)
        {
            return -1;
        }

        value->link -= 1;

        return 0;
    }
}
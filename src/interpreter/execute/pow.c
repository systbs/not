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
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

void 
mpf_to_mpfr(mpfr_t rop, const mpf_t op) {
    mpfr_set_f(rop, op, MPFR_RNDN);
}

void 
mpfr_to_mpf(mpf_t rop, const mpfr_t op) {
    mpq_t q;
    mpq_init(q);
    mpfr_get_q(q, op);
    mpf_set_q(rop, q); 
    mpq_clear(q);
}

void 
mpf_pow(mpf_t result, const mpf_t base, const mpf_t exp) {
    mpfr_t base_fr, exp_fr, result_fr;
    mpfr_init2(base_fr, mpf_get_prec(base));
    mpfr_init2(exp_fr, mpf_get_prec(exp));
    mpfr_init2(result_fr, mpf_get_prec(result));

    mpf_to_mpfr(base_fr, base);
    mpf_to_mpfr(exp_fr, exp);

    mpfr_pow(result_fr, base_fr, exp_fr, MPFR_RNDN);

    mpfr_to_mpf(result, result_fr);

    mpfr_clear(base_fr);
    mpfr_clear(exp_fr);
    mpfr_clear(result_fr);
}

void 
mpf_pow_z(mpf_t result, mpz_t base, mpz_t exp)
{
    mpf_t base_pf, exp_pf;

    mpf_init(base_pf);
    mpf_init(exp_pf);

    mpf_set_z(base_pf, base);
    mpf_set_z(exp_pf, exp);

    mpf_pow(result, base_pf, exp_pf);

    mpf_clear(exp_pf);
    mpf_clear(base_pf);
}

int 
mpf_is_int(mpf_t num)
{
    mpz_t int_part;
    mpf_t temp;

    mpz_init(int_part);
    mpf_init(temp);

    mpz_set_f(int_part, num);
    mpf_set_z(temp, int_part);

    int result = (mpf_cmp(num, temp) == 0);

    mpz_clear(int_part);
    mpf_clear(temp);

    return result;
}


sy_record_t *
sy_execute_pow(sy_node_t *node, sy_strip_t *strip)
{
    if (node->kind == NODE_KIND_POW)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_addative(binary->left, strip);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_addative(binary->right, strip);
        if (right == ERROR)
        {
            return ERROR;
        }

        if (left == NULL)
        {
            if (right == NULL)
            {
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "int8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "int32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "int64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "uint8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "uint16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "uint32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "uint64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "bigint");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "float32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "float64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "bigfloat");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "char");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "null");
                return ERROR;
            }

            return NULL;
        }
        else
        if (left == NAN)
        {
            if (right == NULL)
            {
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "int8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "int32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "int64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "uint8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "uint16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "uint32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "uint64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "bigint");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "float32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "float64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "bigfloat");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "char");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "null");
                return ERROR;
            }

            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_INT8)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int8", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int8", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int8_t *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int8_t *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int8_t *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int8_t *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int8", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int8", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int8", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int8", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int8", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int8", "null");
                return ERROR;
            }

            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_INT16)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int16", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int16", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int16_t *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int16_t *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int16_t *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int16_t *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int16", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int16", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int16", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int16", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int16", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int16", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_INT32)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int32", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int32", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int32_t *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int32_t *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int32_t *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int32_t *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int32", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int32", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int32", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int32", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int32", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int32", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_INT64)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int64", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int64", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int64_t *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int64_t *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(int64_t *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(int64_t *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int64", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int64", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int64", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int64", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int64", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int64", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_UINT8)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint8", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint8", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint8_t *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint8_t *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint8", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint8", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint8", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint8", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint8", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint8", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_UINT16)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint16", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint16", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint16_t *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint16_t *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint16", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint16", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint16", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint16", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint16", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint16", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_UINT32)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint32", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint32", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint32_t *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint32", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint32", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint32", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint32", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint32", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint32", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_UINT64)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint64", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint64", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint32_t *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_ui(num1, (*(uint64_t *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_ui(num1, (*(uint64_t *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint64", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint64", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint64", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint64", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint64", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "uint64", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_BIGINT)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigint", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigint", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                mpf_init(num1);
                
                mpf_set_z(num1, (*(mpz_t *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                mpf_init(num1);
                
                mpf_set_z(num1, (*(mpz_t *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                mpf_init(num1);
                
                mpf_set_z(num1, (*(mpz_t *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set(num1, (*(mpz_t *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigint", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigint", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigint", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigint", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigint", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigint", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_FLOAT32)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float32", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float32", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                mpf_init(num2);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_set_z(num2, (*(mpz_t *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(float *)(left->value)));
                mpf_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float32", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float32", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float32", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float32", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float32", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float32", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_FLOAT64)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float64", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float64", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(double *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(double *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_d(num1, (*(double *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_d(num1, (*(double*)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float64", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float64", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float64", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float64", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float64", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float64", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_BIGFLOAT)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigfloat", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigfloat", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                mpz_init(num1);
                
                mpz_set_f(num1, (*(mpf_t*)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                mpz_init(num1);
                
                mpz_set_f(num1, (*(mpf_t*)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                mpz_init(num1);
                
                mpz_set_f(num1, (*(mpf_t*)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                mpz_init(num1);
                
                mpz_set_f(num1, (*(mpf_t*)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                mpz_init(num1);
                
                mpz_set_f(num1, (*(mpf_t*)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                mpz_init(num1);
                
                mpz_set_f(num1, (*(mpf_t*)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                mpz_init(num1);
                
                mpz_set_f(num1, (*(mpf_t*)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                mpz_init(num1);
                
                mpz_set_f(num1, (*(mpf_t*)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                mpz_init(num1);
                
                mpz_set_f(num1, (*(mpf_t*)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, (*(mpf_t *)(left->value)), num2);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num2, result;

                mpf_init(result);
                
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, (*(mpf_t *)(left->value)), num2);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t result;

                mpf_init(result);

                mpf_pow(result, (*(mpf_t *)(left->value)), (*(mpf_t *)(right->value)));

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpf_t num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_pow(result, (*(mpf_t *)(left->value)), num2);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigfloat", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigfloat", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigfloat", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigfloat", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigfloat", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "bigfloat", "null");
                return ERROR;
            }
            
            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_CHAR)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set_si(num2, (*(int8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set_si(num2, (*(int16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set_si(num2, (*(int32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set_si(num2, (*(int64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set_ui(num2, (*(uint8_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set_ui(num2, (*(uint16_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set_ui(num2, (*(uint32_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set_ui(num2, (*(uint64_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set(num2, (*(mpz_t *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(char *)(left->value)));
                mpf_init_set_d(num2, (*(float *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);


                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                mpf_t num1, num2, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(char *)(left->value)));
                mpf_init_set_d(num2, (*(double *)(right->value)));
                
                mpf_pow(result, num1, num2);
                mpf_clear(num1);
                mpf_clear(num2);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                mpf_t num1, result;

                mpf_init(result);
                
                mpf_init_set_si(num1, (*(char *)(left->value)));
                
                mpf_pow(result, num1, (*(mpf_t *)(right->value)));
                mpf_clear(num1);

                sy_record_t *record = NULL;
                // float
                if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                    record = sy_record_make_float32((float)mpf_get_d(result));
                } 
                // double
                else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                    record = sy_record_make_float64(mpf_get_d(result));
                } 
                // mpf_t
                else {
                    record = sy_record_make_bigfloat_from_mpf(result);
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                mpz_t num1, num2;
                
                mpz_init_set_si(num1, (*(char *)(left->value)));
                mpz_init_set_si(num2, (*(char *)(right->value)));
                
                mpf_t result;
                mpf_init(result);

                mpf_pow_z(result, num1, num2);
                
                mpz_clear(num1);
                mpz_clear(num2);

                sy_record_t *record = NULL;

                if (mpf_is_int(result))
                {
                    mpz_t result_mpz;
                    mpz_init(result_mpz);
                    mpz_set_f(result_mpz, result);

                    // int8_t
                    if (mpz_cmp_si(result_mpz, INT8_MIN) >= 0 && mpz_cmp_si(result_mpz, INT8_MAX) <= 0) {
                        record = sy_record_make_int8((int8_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint8_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT8_MAX) <= 0) {
                        record = sy_record_make_uint8((uint8_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int16_t
                    if (mpz_cmp_si(result_mpz, INT16_MIN) >= 0 && mpz_cmp_si(result_mpz, INT16_MAX) <= 0) {
                        record = sy_record_make_int16((int16_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint16_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT16_MAX) <= 0) {
                        record = sy_record_make_uint16((uint16_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int32_t
                    if (mpz_cmp_si(result_mpz, INT32_MIN) >= 0 && mpz_cmp_si(result_mpz, INT32_MAX) <= 0) {
                        record = sy_record_make_int32((int32_t)mpz_get_si(result_mpz));
                    }
                    else
                    // uint32_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT32_MAX) <= 0) {
                        record = sy_record_make_uint32((uint32_t)mpz_get_ui(result_mpz));
                    }
                    else
                    // int64_t
                    if (mpz_cmp_si(result_mpz, INT64_MIN) >= 0 && mpz_cmp_si(result_mpz, INT64_MAX) <= 0) {
                        record = sy_record_make_int64(mpz_get_si(result_mpz));
                    }
                    else
                    // uint64_t
                    if (mpz_cmp_ui(result_mpz, 0) >= 0 && mpz_cmp_ui(result_mpz, UINT64_MAX) <= 0) {
                        record = sy_record_make_uint64(mpz_get_ui(result_mpz));
                    }
                    // mpz_t
                    else {
                        record = sy_record_make_bigint_from_mpz(result_mpz);
                    }

                    mpz_clear(result_mpz);
                }
                else
                {
                    // float
                    if (mpf_cmp_d(result, FLT_MIN) >= 0 && mpf_cmp_d(result, FLT_MAX) <= 0) {
                        record = sy_record_make_float32((float)mpf_get_d(result));
                    } 
                    // double
                    else if (mpf_cmp_d(result, DBL_MIN) >= 0 && mpf_cmp_d(result, DBL_MAX) <= 0) {
                        record = sy_record_make_float64(mpf_get_d(result));
                    } 
                    // mpf_t
                    else {
                        record = sy_record_make_bigfloat_from_mpf(result);
                    }
                }

                mpf_clear(result);

                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                return record;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "null");
                return ERROR;
            }

            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_STRING)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "int8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "int16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "int32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "int64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "uint8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "uint16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "uint32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "uint64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "bigint");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "float32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "float64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "bigfloat");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "char");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "string");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "null");
                return ERROR;
            }

            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_OBJECT)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "int8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "int16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "int32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "int64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "uint8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "uint16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "uint32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "uint64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "bigint");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "float32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "float64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "bigfloat");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "char");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "null");
                return ERROR;
            }

            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_TUPLE)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "int8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "int16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "int32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "int64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "uint8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "uint16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "uint32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "uint64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "bigint");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "float32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "float64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "bigfloat");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "char");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "null");
                return ERROR;
            }

            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_TYPE)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "int8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "int16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "int32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "int64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "uint8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "uint16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "uint32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "uint64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "bigint");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "float32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "float64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "bigfloat");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "char");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "null");
                return ERROR;
            }

            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_STRUCT)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "int8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "int16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "int32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "int64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "uint8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "uint16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "uint32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "uint64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "bigint");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "float32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "float64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "bigfloat");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "char");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "struct", "null");
                return ERROR;
            }

            return NULL;
        }
        else
        if (left->kind == RECORD_KIND_NULL)
        {
            if (right == NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "undefined");
                return ERROR;
            }
            else
            if (right == NAN)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "nan");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "int8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "int16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "int32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_INT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "int64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT8)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "uint8");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT16)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "uint16");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "uint32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_UINT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "uint64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGINT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "bigint");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT32)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "float32");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_FLOAT64)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "float64");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_BIGFLOAT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "bigfloat");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_CHAR)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "char");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRING)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "null");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_OBJECT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "object");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TUPLE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "tuple");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_TYPE)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "type");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_STRUCT)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "struct");
                return ERROR;
            }
            else
            if (right->kind == RECORD_KIND_NULL)
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }

                sy_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "null");
                return ERROR;
            }

            return NULL;
        }

        return NULL;
    }
    else
    {
        return sy_execute_prefix(node, strip);
    }
}

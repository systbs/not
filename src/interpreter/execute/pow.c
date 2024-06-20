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

#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

void mpf_to_mpfr(mpfr_t rop, const mpf_t op)
{
    mpfr_set_f(rop, op, MPFR_RNDN);
}

void mpfr_to_mpf(mpf_t rop, const mpfr_t op)
{
    mpq_t q;
    mpq_init(q);
    mpfr_get_q(q, op);
    mpf_set_q(rop, q);
    mpq_clear(q);
}

void mpf_pow(mpf_t result, const mpf_t base, const mpf_t exp)
{
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

void mpf_pow_z(mpf_t result, mpz_t base, mpz_t exp)
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

int mpf_is_int(mpf_t num)
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

not_record_t *
not_power_pow(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2;

            mpz_init_set(num1, (*(mpz_t *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NOT_PTR_NULL;

            if (mpf_is_int(result))
            {
                mpz_t result_mpz;
                mpz_init(result_mpz);
                mpz_set_f(result_mpz, result);

                record = not_record_make_int_from_z(result_mpz);

                mpz_clear(result_mpz);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t num1, result;

            mpf_init(result);
            mpf_init(num1);

            mpf_set_z(num1, (*(mpz_t *)(left->value)));

            mpf_pow(result, num1, (*(mpf_t *)(right->value)));
            mpf_clear(num1);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num1, num2;

            mpz_init_set(num1, (*(mpz_t *)(left->value)));
            mpz_init_set_si(num2, (*(char *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NOT_PTR_NULL;

            if (mpf_is_int(result))
            {
                mpz_t result_mpz;
                mpz_init(result_mpz);
                mpz_set_f(result_mpz, result);

                record = not_record_make_int_from_z(result_mpz);

                mpz_clear(result_mpz);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2;
            mpz_init(num1);

            mpz_set_f(num1, (*(mpf_t *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NOT_PTR_NULL;

            if (mpf_is_int(result))
            {
                mpz_t result_mpz;
                mpz_init(result_mpz);
                mpz_set_f(result_mpz, result);

                record = not_record_make_int_from_z(result_mpz);

                mpz_clear(result_mpz);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t result;

            mpf_init(result);

            mpf_pow(result, (*(mpf_t *)(left->value)), (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpf_t num2, result;

            mpf_init(result);

            mpf_init_set_si(num2, (*(char *)(right->value)));

            mpf_pow(result, (*(mpf_t *)(left->value)), num2);
            mpf_clear(num2);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2;

            mpz_init_set_si(num1, (*(char *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NOT_PTR_NULL;

            if (mpf_is_int(result))
            {
                mpz_t result_mpz;
                mpz_init(result_mpz);
                mpz_set_f(result_mpz, result);

                record = not_record_make_int_from_z(result_mpz);

                mpz_clear(result_mpz);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t num1, result;

            mpf_init(result);

            mpf_init_set_si(num1, (*(char *)(left->value)));

            mpf_pow(result, num1, (*(mpf_t *)(right->value)));
            mpf_clear(num1);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num1, num2;

            mpz_init_set_si(num1, (*(char *)(left->value)));
            mpz_init_set_si(num2, (*(char *)(right->value)));

            mpf_t result;
            mpf_init(result);

            mpf_pow_z(result, num1, num2);

            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NOT_PTR_NULL;

            if (mpf_is_int(result))
            {
                mpz_t result_mpz;
                mpz_init(result_mpz);
                mpz_set_f(result_mpz, result);

                record = not_record_make_int_from_z(result_mpz);

                mpz_clear(result_mpz);
            }
            else
            {
                record = not_record_make_float_from_f(result);
            }

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, "**", applicant);
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "null");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }

    return not_record_make_undefined();
}

not_record_t *
not_power(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_POW)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_power(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_prefix(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_power_pow(node, left, right, applicant);

        if (not_record_link_decrease(left) < 0)
        {
            return NOT_PTR_ERROR;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return NOT_PTR_ERROR;
        }

        return record;
    }
    else
    {
        return not_prefix(node, strip, applicant, origin);
    }
}

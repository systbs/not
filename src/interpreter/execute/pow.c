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
#include "../garbage.h"
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
not_execute_pow(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "undefiend", "null");
            return ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "nan", "null");
            return ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "nan");
            return ERROR;
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

            not_record_t *record = NULL;

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

            not_record_t *record = NULL;

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
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "int", "null");
            return ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "nan");
            return ERROR;
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

            not_record_t *record = NULL;

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
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "float", "null");
            return ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "nan");
            return ERROR;
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

            not_record_t *record = NULL;

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

            not_record_t *record = NULL;

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
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "char", "null");
            return ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "string", "null");
            return ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "object", "null");
            return ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "tuple", "null");
            return ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "type", "null");
            return ERROR;
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
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "null");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for **: '%s' and '%s'", "null", "null");
            return ERROR;
        }

        return not_record_make_undefined();
    }

    return not_record_make_undefined();
}

not_record_t *
not_execute_power(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_POW)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_execute_power(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        not_record_t *right = not_execute_prefix(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            not_record_link_decrease(left);
            return ERROR;
        }

        not_record_t *record = not_execute_pow(node, left, right, applicant);

        if (not_record_link_decrease(left) < 0)
        {
            return ERROR;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return ERROR;
        }

        return record;
    }
    else
    {
        return not_execute_prefix(node, strip, applicant, origin);
    }
}

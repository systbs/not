#pragma once
#ifndef __EXECUTE_H__
#define __EXECUTE_H__ 1

int32_t
sy_execute_id_cmp(sy_node_t *n1, sy_node_t *n2);

int32_t
sy_execute_id_strcmp(sy_node_t *n1, const char *name);

int32_t
sy_execute_type_check_by_type(sy_record_t *record_type1, sy_record_t *record_type2, sy_strip_t *strip, sy_node_t *applicant);

int32_t
sy_execute_value_check_by_type(sy_record_t *record_value, sy_record_t *record_type, sy_strip_t *strip, sy_node_t *applicant);

sy_record_t *
sy_execute_value_casting_by_type(sy_record_t *record_value, sy_record_t *record_type, sy_strip_t *strip, sy_node_t *applicant);

int32_t 
sy_execute_run_fun(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant);

int32_t 
sy_execute_run_lambda(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant);

int32_t
sy_execute_parameters_substitute(sy_node_t *base, sy_node_t *scope, sy_strip_t *strip, sy_node_t *parameters, sy_node_t *arguments, sy_node_t *applicant);

int32_t 
sy_execute_parameters_check_by_one_argument(sy_strip_t *strip, sy_node_t *parameters, sy_record_t *arg, sy_node_t *applicant);

int32_t 
sy_execute_parameters_substitute_by_one_argument(sy_node_t *base, sy_node_t *scope, sy_strip_t *strip, sy_node_t *parameters, sy_record_t *arg, sy_node_t *applicant);

sy_record_t *
sy_execute_call_for_operator_by_one_argument(sy_node_t *base, sy_record_t *content, sy_record_t *arg, const char *operator, sy_node_t *applicant);

int32_t 
sy_execute_run(sy_node_t *root);

sy_record_t *
sy_execute_logical_or(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_logical_and(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_bitwise_or(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_bitwise_xor(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_bitwise_and(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_equality(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_relational(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_shifting(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_addative(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_multipicative(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_power(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_prefix(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_call(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_array(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_pseudonym(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_attribute(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_primary(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_postfix(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_expression(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

int32_t
sy_execute_assign(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_id(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_number(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_char(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_string(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_null(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_kint8(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kint16(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kint32(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kint64(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kuint8(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kuint16(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kuint32(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kuint64(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kint(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kfloat32(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kfloat64(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kfloat(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kchar(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kstring(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_self(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_this(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_lambda(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_parenthesis(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);

sy_record_t *
sy_execute_pow(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_mul(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_div(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_mod(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_epi(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_shr(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_shl(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_plus(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_minus(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_lor(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_land(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_xor(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_or(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_and(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_lt(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_le(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_eq(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_attribute_from_struct(sy_node_t *node, sy_strip_t *strip, sy_node_t *left, sy_node_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_attribute_from_type(sy_node_t *node, sy_strip_t *strip, sy_node_t *left, sy_node_t *right, sy_node_t *applicant);

sy_record_t *
sy_execute_eq(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant);

char *
record_to_string(sy_record_t *record, char *previous_buf);

#endif
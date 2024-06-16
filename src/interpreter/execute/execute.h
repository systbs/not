#pragma once
#ifndef __EXECUTE_H__
#define __EXECUTE_H__ 1

int32_t
not_execute_id_cmp(not_node_t *n1, not_node_t *n2);

int32_t
not_execute_id_strcmp(not_node_t *n1, const char *name);

int32_t
not_execute_type_check_by_type(not_node_t *node, not_record_t *record_type1, not_record_t *record_type2, not_strip_t *strip, not_node_t *applicant);

int32_t
not_execute_value_check_by_type(not_node_t *node, not_record_t *record_value, not_record_t *record_type, not_strip_t *strip, not_node_t *applicant);

not_record_t *
not_execute_value_casting_by_type(not_node_t *node, not_record_t *record_value, not_record_t *record_type, not_strip_t *strip, not_node_t *applicant);

int32_t
not_execute_run_fun(not_node_t *node, not_strip_t *strip, not_node_t *applicant);

int32_t
not_execute_run_lambda(not_node_t *node, not_strip_t *strip, not_node_t *applicant);

int32_t
not_call_parameters_subs(not_node_t *base, not_node_t *scope, not_strip_t *strip, not_node_t *parameters, not_node_t *arguments, not_node_t *applicant);

int32_t
not_call_parameters_check_by_one_arg(not_node_t *node, not_strip_t *strip, not_node_t *parameters, not_record_t *arg, not_node_t *applicant);

int32_t
not_call_parameters_subs_by_one_arg(not_node_t *base, not_node_t *scope, not_strip_t *strip, not_node_t *parameters, not_record_t *arg, not_node_t *applicant);

not_record_t *
not_call_operator_by_one_arg(not_node_t *base, not_record_t *content, not_record_t *arg, const char *operator, not_node_t * applicant);

int32_t
not_execute_run(not_node_t *root);

not_record_t *
not_execute_logical_or(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_logical_and(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_bitwise_or(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_bitwise_xor(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_bitwise_and(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_equality(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_relational(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_shifting(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_addative(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_multipicative(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_power(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_prefix(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_call(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_array(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_pseudonym(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_attribute(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_primary(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_postfix(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_instanceof(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_conditional(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_expression(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

int32_t
not_execute_assign(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_id(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_number(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_char(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_string(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_null(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_kint(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);
not_record_t *
not_execute_kfloat(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);
not_record_t *
not_execute_kchar(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);
not_record_t *
not_execute_kstring(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);
not_record_t *
not_execute_lambda(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);
not_record_t *
not_execute_parenthesis(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_execute_pow(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_mul(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_div(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_mod(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_epi(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_shr(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_shl(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_plus(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_minus(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_lor(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_land(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_xor(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_or(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_and(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_lt(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_le(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_eq(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_execute_attribute_from_struct(not_node_t *node, not_strip_t *strip, not_node_t *left, not_node_t *right, not_node_t *applicant);

not_record_t *
not_execute_attribute_from_type(not_node_t *node, not_strip_t *strip, not_node_t *left, not_node_t *right, not_node_t *applicant);

not_record_t *
not_execute_eq(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

int32_t
not_execute_truthy(not_record_t *left);

#endif
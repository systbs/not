#pragma once
#ifndef __EXECUTE_H__
#define __EXECUTE_H__ 1

int32_t
not_execute_type_check_by_type(not_node_t *node, not_record_t *record_type1, not_record_t *record_type2);

int32_t
not_execute_value_check_by_type(not_node_t *node, not_record_t *record_value, not_record_t *record_type);

not_record_t *
not_execute_value_casting_by_type(not_node_t *node, not_record_t *record_value, not_record_t *record_type);

int32_t
not_execute_value_check_by_value(not_node_t *node, not_record_t *record_value1, not_record_t *record_value2);

int32_t
not_execute_fun(not_node_t *node, not_strip_t *strip, not_node_t *applicant);

int32_t
not_execute_lambda(not_node_t *node, not_strip_t *strip, not_node_t *applicant);

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
not_logical_or(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_logical_and(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_bitwise_or(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_bitwise_xor(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_bitwise_and(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_equality(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_relational(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_shifting(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_addative(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_multipicative(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_power(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_prefix(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_call(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_array(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_pseudonym(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_attribute(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_primary(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_postfix(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_instanceof(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_conditional(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_expression(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

int32_t
not_execute_assign(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin);

not_record_t *
not_power_pow(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_multipicative_mul(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_multipicative_div(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_multipicative_mod(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_multipicative_epi(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_shifting_shr(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_shifting_shl(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_addative_plus(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_addative_minus(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

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
not_relational_lt(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_relational_le(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_equality_eq(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

not_record_t *
not_attribute_from_struct(not_node_t *node, not_strip_t *strip, not_node_t *left, not_node_t *right, not_node_t *applicant);

not_record_t *
not_attribute_from_type(not_node_t *node, not_strip_t *strip, not_node_t *left, not_node_t *right, not_node_t *applicant);

not_record_t *
not_equality_eq(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant);

int32_t
not_execute_truthy(not_record_t *left);

#endif
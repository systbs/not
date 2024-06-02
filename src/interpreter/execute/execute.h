#pragma once
#ifndef __EXECUTE_H__
#define __EXECUTE_H__ 1

int32_t
sy_execute_id_cmp(sy_node_t *n1, sy_node_t *n2);

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
sy_execute_kbigint(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kfloat32(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kfloat64(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
sy_record_t *
sy_execute_kbigfloat(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin);
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
sy_execute_pow(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_mul(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_div(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_mod(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_epi(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_shr(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_shl(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_plus(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_minus(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_lor(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_land(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_xor(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_or(sy_node_t *node, sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_and(sy_node_t *node, sy_record_t *left, sy_record_t *right);

int32_t
sy_execute_truthy_lt(sy_record_t *left, sy_record_t *right);

int32_t
sy_execute_truthy_le(sy_record_t *left, sy_record_t *right);

int32_t
sy_execute_truthy_eq(sy_record_t *left, sy_record_t *right);

#endif
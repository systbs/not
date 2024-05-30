#pragma once
#ifndef __EXECUTE_H__
#define __EXECUTE_H__ 1

int32_t
sy_execute_id_cmp(sy_node_t *n1, sy_node_t *n2);

int32_t 
sy_execute_run(sy_node_t *root);

sy_record_t *
sy_execute_logical_or(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_logical_and(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_bitwise_or(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_bitwise_xor(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_bitwise_and(sy_node_t *node, sy_strip_t *strip);

int32_t
sy_execute_truthy_equal(sy_record_t *left, sy_record_t *right);

sy_record_t *
sy_execute_equality(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_relational(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_shifting(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_addative(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_multipicative(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_pow(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_prefix(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_call(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_array(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_pseudonym(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_attribute(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_primary(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_postfix(sy_node_t *node, sy_strip_t *strip);

sy_record_t *
sy_execute_expression(sy_node_t *node, sy_strip_t *strip);

int32_t
sy_execute_assign(sy_node_t *node, sy_strip_t *strip);

#endif
#pragma once
#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

typedef enum semantic_flag {
    SEMANTIC_FLAG_NONE = 0 << 0
} semantic_flag_t;

// semantic.c
error_t *
semantic_error(program_t *program, node_t *node, const char *format, ...);

int32_t
semantic_idcmp(node_t *n1, node_t *n2);

int32_t
semantic_idstrcmp(node_t *n1, char *name);

int32_t
semantic_module(program_t *program, node_t *node);

int32_t
semantic_run(program_t *program, node_t *root);


// subset.c
int32_t
semantic_tequality(program_t *program, node_t *n1, node_t *n2);

int32_t
semantic_subset(program_t *program, node_t *n1, node_t *n2);


// equal.c
int32_t
semantic_eqaul_gsgs(program_t *program, node_t *ngs1, node_t *ngs2);

int32_t
semantic_eqaul_psps(program_t *program, node_t *nps1, node_t *nps2);

int32_t
semantic_eqaul_psas(program_t *program, node_t *nps1, node_t *nas2);

int32_t
semantic_eqaul_gsfs(program_t *program, node_t *ngs1, node_t *nfs2);

int32_t
semantic_eqaul_psns(program_t *program, node_t *nps1, node_t *nns2);

int32_t
semantic_eqaul_otos(program_t *program, node_t *not1, node_t *nos2);

int32_t
semantic_eqaul_osos(program_t *program, node_t *nos1, node_t *nos2);


// make.c
node_t *
semantic_make_field(program_t *program, node_t *arg);

node_t *
semantic_make_fields(program_t *program, node_t *parent, uint64_t n, ...);

node_t *
semantic_make_argument(program_t *program, node_t *arg);

node_t *
semantic_make_arguments(program_t *program, node_t *parent, uint64_t n, ...);


// primary.c
int32_t
semantic_primary(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag);


// pseudonym.c
int32_t
semantic_pseudonym(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag);


// attribute.c
int32_t
semantic_attribute(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag);


// call.c
int32_t
semantic_call(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag);


// expression.c
int32_t
semantic_resolve(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag);

int32_t
semantic_postfix(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag);

int32_t
semantic_expression(program_t *program, node_t *node, list_t *response, uint64_t flag);

int32_t
semantic_assignment(program_t *program, node_t *node1, node_t *node2, node_t *major, uint64_t flag);

int32_t
semantic_assign(program_t *program, node_t *node, uint64_t flag);

// helper.c
int32_t
semantic_gresolve(program_t *program, node_t *node, list_t *response, uint64_t flag);

int32_t
semantic_hresolve(program_t *program, node_t *node, list_t *response, uint64_t flag);

int32_t
semantic_vresolve(program_t *program, node_t *node, list_t *response, uint64_t flag);

#endif
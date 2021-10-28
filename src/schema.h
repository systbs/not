#pragma once

typedef enum schema_type {
	SCHEMA_DUP,
	SCHEMA_PRIMARY
} schema_type_t;

typedef struct schema {
	char *identifier;

	schema_type_t type;

	iarray_t *start;
	iarray_t *end;

	table_t *branches;
	struct schema *subschema;


	table_t *parameters;
	table_t *variables;

	table_t *packet;

	table_t *frame;
	table_t *extends;
	table_t *stack;

	table_t *duplicate;

	struct schema *parent;
} schema_t;


schema_t *
schema_create(schema_t *parent);

schema_t *
schema_duplicate(schema_t *source);

schema_t *
schema_fpt(table_t *tbl, char *identifier);

schema_t *
schema_branches(schema_t *schema, char *identifier);

variable_t *
schema_variable(schema_t *schema, char *identifier);

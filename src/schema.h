#pragma once

typedef struct schema {
	char_t *identifier;

	iarray_t *start;
	iarray_t *end;

	table_t *extends;
	table_t *branches;
	table_t *variables;
	table_t *parameters;

	struct schema *parent;
} schema_t;


schema_t *
schema_create(schema_t *parent);

schema_t *
schema_fpt(table_t *tbl, char *identifier);

schema_t *
schema_branches(schema_t *schema, char *identifier);

variable_t *
schema_variable(schema_t *schema, char *identifier);

#pragma once

typedef struct schema {
	char_t *identifier;

	ilist_t *start;
	ilist_t *end;

	list_t *extends;
	list_t *branches;
	list_t *variables;
	list_t *parameters;

	void_t *root;

	struct schema *parent;
} schema_t;


schema_t *
schema_create(schema_t *parent);

schema_t *
schema_fpt(list_t *lst, char *identifier);

schema_t *
schema_branches(schema_t *schema, char *identifier);

variable_t *
schema_find_variable(schema_t *schema, char *identifier);

#pragma once

typedef struct layout {
    char *identifier;

    schema_t *schema;

    table_t *extends;
	table_t *parameters;
	table_t *variables;
	table_t *frame;

	object_t *object;

	struct layout *parent;
} layout_t;

layout_t *
layout_create(schema_t *schema);

layout_t *
layout_fpt(table_t *tbl, char *identifier);

variable_t *
layout_variable(layout_t *layout, char *identifier);

variable_t *
layout_fcnt(layout_t *layout, object_t *object);
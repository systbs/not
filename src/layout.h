#pragma once

typedef struct layout {
    char_t *identifier;

    schema_t *schema;

    list_t *extends;
	list_t *parameters;
	list_t *variables;
	list_t *frame;
	list_t *scope;

	object_t *object;

	struct layout *parent;
} layout_t;

layout_t *
layout_create(schema_t *schema);

layout_t *
layout_fpt(list_t *lst, char *identifier);

variable_t *
layout_variable(layout_t *layout, char *identifier);

variable_t *
layout_vwp(layout_t *layout, char *identifier);

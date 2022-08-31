#pragma once

typedef struct variable {
	char_t *identifier;
	object_t *object;
} variable_t;

variable_t *
variable_findlst(list_t *variables, char *identifier);

variable_t *
variable_fcnt(list_t *variables, object_t *object);

variable_t *
variable_define(char *identifier);

void *
variable_content(variable_t *var);

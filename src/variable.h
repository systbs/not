#pragma once

typedef struct variable {
	char *identifier;
	object_t *object;
} variable_t;

variable_t *
variable_findlst(table_t *variables, char *identifier);

variable_t *
variable_define(char *identifier);

void *
variable_content(variable_t *var);
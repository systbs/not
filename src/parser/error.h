#pragma once
#ifndef __ERRORS_H__

typedef struct error {
    position_t position;
    char *message;
} error_t;

error_t *
error_create(position_t position, char *message);

#endif
#pragma once
#ifndef __ERRORS_H__

typedef struct error {
    position_t position;
    char *message;

    node_t *origin;
} error_t;

error_t *
error_create(position_t position, char *message);

#endif
#pragma once
#ifndef __RESPONSE_H__
#define __RESPONSE_H__

typedef struct iresponse {
    struct iresponse *previous;
    struct iresponse *next;
    uint64_t flags;
    void *value;
} iresponse_t;

typedef struct response {
    iresponse_t *begin;
    iresponse_t *end;
} response_t;

response_t *
response_apply(response_t *res);

response_t *
response_create();

int
response_isempty(response_t *res);

iresponse_t*
response_next(iresponse_t *current);

iresponse_t*
response_previous(iresponse_t *current);

uint64_t
response_count(response_t *res);

int32_t
response_query(response_t *res, int (*f)(iresponse_t*));

void
response_destroy(response_t *res);

void
response_link(response_t *res, iresponse_t *current, iresponse_t *it);

void
response_unlink(response_t *res, iresponse_t* it);

void
response_sort(response_t *res, int (*f)(iresponse_t *, iresponse_t *));

void
response_unlink_by(response_t *res, int (*f)(iresponse_t *));

void
response_clear(response_t *res);

iresponse_t*
response_rpop(response_t *res);

iresponse_t *
response_rpush(response_t *res, void *value);

iresponse_t*
response_lpop(response_t *res);

iresponse_t *
response_lpush(response_t *res, void *value);

iresponse_t *
response_first(response_t *res);

iresponse_t *
response_last(response_t *res);

#endif
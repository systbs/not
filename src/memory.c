#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "types/types.h"
#include "memory.h"


typedef long Align;

union header {
  struct {
    union header *ptr;
    size_t size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void
sy_memory_arena_free(void *ap)
{
    Header *bp, *p;

    bp = (Header*)ap - 1;
    for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
        break;
    if(bp + bp->s.size == p->s.ptr)
    {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } 
    else
        bp->s.ptr = p->s.ptr;

    if(p + p->s.size == bp)
    {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } 
    else
        p->s.ptr = bp;
    freep = p;
}

static Header*
sy_memory_arena_morecore(size_t nu)
{
    char *p;
    Header *hp;

    if(nu < 4096)
        nu = 4096;
    p = sbrk(nu * sizeof(Header));
    if(p == (char*)-1)
        return 0;
    hp = (Header*)p;
    hp->s.size = nu;
    sy_memory_arena_free((void*)(hp + 1));
    return freep;
}

void *
sy_memory_arena_malloc(size_t nbytes)
{
    Header *p, *prevp;
    size_t nunits;

    nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
    if((prevp = freep) == 0)
    {
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }
    for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr)
    {
        if(p->s.size >= nunits)
        {
            if(p->s.size == nunits)
                prevp->s.ptr = p->s.ptr;
            else
            {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void*)(p + 1);
        }
        if(p == freep)
            if((p = sy_memory_arena_morecore(nunits)) == 0)
                return 0;
    }
}


void *
sy_memory_calloc(size_t num, size_t size)
{
#ifdef _WIN32
    return calloc(num, size);
#else
    size_t total = num * size;
    void *ptr = sy_memory_arena_malloc(total);
    if (ptr == NULL)
    {
        return NULL;
    }
    memset(ptr, 0, total);
    return ptr;
#endif
}

void *
sy_memory_malloc(size_t size)
{
#ifdef _WIN32
    return malloc(size);
#else
    return sy_memory_arena_malloc(size);
#endif
}

void *
sy_memory_realloc(void *ptr, size_t size)
{
#ifdef _WIN32
    return realloc(ptr, size);
#else
    sy_memory_arena_free(ptr);
    void *result = sy_memory_arena_malloc(size);
    if (result == NULL)
    {
        return NULL;
    }
    return result;
#endif
}

void
sy_memory_free(void *ptr)
{
#ifdef _WIN32
    return free(size);
#else
    sy_memory_arena_free(ptr);
#endif
}
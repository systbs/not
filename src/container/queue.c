#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../types/types.h"
#include "../memory.h"
#include "../error.h"
#include "queue.h"

sy_queue_t *
sy_queue_apply(sy_queue_t *queue)
{
	sy_queue_entry_t *entry = (sy_queue_entry_t *)sy_memory_calloc(1, sizeof(sy_queue_entry_t));
	if(!entry)
	{
		sy_error_no_memory();
		return ERROR;
	}

	entry->next = entry->previous = entry;
	queue->end = queue->begin = entry;

	return queue;
}

sy_queue_t *
sy_queue_create()
{
	sy_queue_t *queue = (sy_queue_t *)sy_memory_calloc(1, sizeof(sy_queue_t));
	if(!queue)
	{
		sy_error_no_memory();
		return ERROR;
	}

	return sy_queue_apply(queue);
}

int32_t
sy_queue_is_empty(sy_queue_t *queue)
{
	return (queue->begin == queue->end);
}

uint64_t
sy_queue_count(sy_queue_t *queue)
{
	uint64_t cnt = 0;
	sy_queue_entry_t *b;
	for(b = queue->begin; b && (b != queue->end); b = b->next)
	{
		cnt++;
	}
	return cnt;
}

int32_t
sy_queue_query(sy_queue_t *queue, int (*f)(sy_queue_entry_t*))
{
	if (sy_queue_is_empty(queue))
	{
		return 0;
	}

	sy_queue_entry_t *b, *n;
	for(b = queue->begin; b && (b != queue->end); b = n)
	{
		n = b->next;
		if(!(*f)(b))
		{
			return 0;
		}
	}

	return 1;
}

void
sy_queue_destroy(sy_queue_t *queue)
{
  sy_queue_clear(queue);
  sy_memory_free (queue);
}

void
sy_queue_link(sy_queue_t *queue, sy_queue_entry_t *current, sy_queue_entry_t *entry)
{
	entry->next = current;
	entry->previous = current->previous;

	if (current->previous)
	{
		current->previous->next = entry;
	}
	
	current->previous = entry;

	if (queue->begin == current)
	{
		queue->begin = entry;
	}
}

void
sy_queue_unlink(sy_queue_t *queue, sy_queue_entry_t* entry)
{
	if (entry == queue->end)
	{
		return;
	}

	if (entry == queue->begin)
	{
		queue->begin = entry->next;
	}

	if (entry->next)
	{
		entry->next->previous = entry->previous;
	}

	if (entry->previous)
	{
		entry->previous->next = entry->next;
	}
}

void
sy_queue_sort(sy_queue_t *queue, int (*f)(sy_queue_entry_t *, sy_queue_entry_t *))
{
	sy_queue_entry_t *b, *n;
	for(b = queue->begin; b != queue->end; b = n)
	{
		n = b->next;
		if(n != queue->end)
		{
			if((*f)(b, n))
			{
				sy_queue_unlink(queue, b);
				sy_queue_link(queue, n, b);
			}
		}
	}
}

void
sy_queue_clear(sy_queue_t *queue)
{
	sy_queue_entry_t *b, *n;
	for(b = queue->begin; b != queue->end; b = n){
		n = b->next;
		sy_queue_unlink(queue, b);
		sy_memory_free(b);
	}
}

sy_queue_entry_t *
sy_queue_right_pop(sy_queue_t *queue)
{
	if(sy_queue_is_empty(queue)){
		return NULL;
	}
	sy_queue_entry_t *entry = queue->end->previous;
	sy_queue_unlink(queue, entry);
	return entry;
}

sy_queue_entry_t *
sy_queue_right_push(sy_queue_t *queue, void *value)
{
	sy_queue_entry_t *entry = (sy_queue_entry_t *)sy_memory_calloc(1, sizeof(sy_queue_entry_t));
	if(!entry)
	{
		sy_error_no_memory();
		return ERROR;
	}

	entry->value = value;

	sy_queue_link(queue, queue->end, entry);
	return entry;
}

sy_queue_entry_t *
sy_queue_left_pop(sy_queue_t *queue)
{
	sy_queue_entry_t *entry = queue->begin;
	sy_queue_unlink(queue, entry);
	return entry;
}

sy_queue_entry_t *
sy_queue_left_push(sy_queue_t *queue, void *value)
{
  sy_queue_entry_t *entry = (sy_queue_entry_t *)sy_memory_calloc(1, sizeof(sy_queue_entry_t));
  if(!entry)
  {
	sy_error_no_memory();
    return ERROR;
  }
  entry->value = value;

  sy_queue_link(queue, queue->begin, entry);
  return entry;
}

sy_queue_entry_t *
sy_queue_insert(sy_queue_t *queue, sy_queue_entry_t *index, void *value)
{
	sy_queue_entry_t *entry = (sy_queue_entry_t *)sy_memory_calloc(1, sizeof(sy_queue_entry_t));
	if(!entry)
	{
		sy_error_no_memory();
		return ERROR;
	}

	entry->value = value;

	sy_queue_link(queue, index, entry);
	return entry;
}
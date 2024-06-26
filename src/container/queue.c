#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../types/types.h"
#include "../memory.h"
#include "../error.h"
#include "queue.h"

not_queue_t *
not_queue_apply(not_queue_t *queue)
{
	not_queue_entry_t *entry = (not_queue_entry_t *)not_memory_calloc(1, sizeof(not_queue_entry_t));
	if (!entry)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	entry->next = entry->previous = entry;
	queue->end = queue->begin = entry;

	return queue;
}

not_queue_t *
not_queue_create()
{
	not_queue_t *queue = (not_queue_t *)not_memory_calloc(1, sizeof(not_queue_t));
	if (!queue)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	return not_queue_apply(queue);
}

int32_t
not_queue_is_empty(not_queue_t *queue)
{
	return (queue->begin == queue->end);
}

uint64_t
not_queue_count(not_queue_t *queue)
{
	uint64_t cnt = 0;
	not_queue_entry_t *b;
	for (b = queue->begin; b && (b != queue->end); b = b->next)
	{
		cnt++;
	}
	return cnt;
}

int32_t
not_queue_query(not_queue_t *queue, int (*f)(not_queue_entry_t *))
{
	if (not_queue_is_empty(queue))
	{
		return 0;
	}

	not_queue_entry_t *b, *n;
	for (b = queue->begin; b && (b != queue->end); b = n)
	{
		n = b->next;
		if (!(*f)(b))
		{
			return 0;
		}
	}

	return 1;
}

void not_queue_destroy(not_queue_t *queue)
{
	not_queue_clear(queue);
	not_memory_free(queue->end);
	not_memory_free(queue);
}

void not_queue_link(not_queue_t *queue, not_queue_entry_t *current, not_queue_entry_t *entry)
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

void not_queue_unlink(not_queue_t *queue, not_queue_entry_t *entry)
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

	entry->next = entry->previous = NULL;
}

void not_queue_sort(not_queue_t *queue, int (*f)(not_queue_entry_t *, not_queue_entry_t *))
{
	not_queue_entry_t *b, *n;
	for (b = queue->begin; b != queue->end; b = n)
	{
		n = b->next;
		if (n != queue->end)
		{
			if ((*f)(b, n))
			{
				not_queue_unlink(queue, b);
				not_queue_link(queue, n, b);
			}
		}
	}
}

void not_queue_clear(not_queue_t *queue)
{
	not_queue_entry_t *b, *n;
	for (b = queue->begin; b != queue->end; b = n)
	{
		n = b->next;
		not_queue_unlink(queue, b);
		not_memory_free(b);
	}
}

not_queue_entry_t *
not_queue_right_pop(not_queue_t *queue)
{
	if (not_queue_is_empty(queue))
	{
		return NULL;
	}
	not_queue_entry_t *entry = queue->end->previous;
	not_queue_unlink(queue, entry);
	return entry;
}

not_queue_entry_t *
not_queue_right_push(not_queue_t *queue, void *value)
{
	not_queue_entry_t *entry = (not_queue_entry_t *)not_memory_calloc(1, sizeof(not_queue_entry_t));
	if (!entry)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	entry->value = value;

	not_queue_link(queue, queue->end, entry);
	return entry;
}

not_queue_entry_t *
not_queue_left_pop(not_queue_t *queue)
{
	not_queue_entry_t *entry = queue->begin;
	not_queue_unlink(queue, entry);
	return entry;
}

not_queue_entry_t *
not_queue_left_push(not_queue_t *queue, void *value)
{
	not_queue_entry_t *entry = (not_queue_entry_t *)not_memory_calloc(1, sizeof(not_queue_entry_t));
	if (!entry)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}
	entry->value = value;

	not_queue_link(queue, queue->begin, entry);
	return entry;
}

not_queue_entry_t *
not_queue_insert(not_queue_t *queue, not_queue_entry_t *index, void *value)
{
	not_queue_entry_t *entry = (not_queue_entry_t *)not_memory_calloc(1, sizeof(not_queue_entry_t));
	if (!entry)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	entry->value = value;

	not_queue_link(queue, index, entry);
	return entry;
}
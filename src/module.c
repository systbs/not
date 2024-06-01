#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "types/types.h"
#include "utils/path.h"
#include "container/queue.h"
#include "token/position.h"
#include "token/token.h"
#include "scanner/scanner.h"
#include "ast/node.h"
#include "parser/syntax/syntax.h"
#include "parser/semantic/semantic.h"
#include "error.h"
#include "mutex.h"
#include "config.h"
#include "memory.h"
#include "module.h"
#include "interpreter.h"
#include "thread.h"

SyModule_t base_module;

int32_t 
sy_module_init()
{
    SyModule_t *module = sy_module_get();
    if (sy_mutex_init(&module->lock) < 0)
    {
        sy_error_system("'%s' could not initialize the lock", "SyModule.lock");
        return -1;
    }

    module->begin = NULL;

    return 0;
}

SyModule_t *
sy_module_get()
{
    return &base_module;
}

sy_module_entry_t_t *
sy_module_get_entry_by_path(const char *path)
{
    SyModule_t *module = sy_module_get();

    if (sy_mutex_lock(&module->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "SyModule.lock");
        return ERROR;
    }

    for (sy_module_entry_t_t *item = module->begin; item != NULL; item = item->next)
    {
        if (strcmp(item->path, path) == 0)
        {
            if (sy_mutex_unlock(&module->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "SyModule.lock");
                return ERROR;
            }
            return item;
        }
    }

    if (sy_mutex_unlock(&module->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "SyModule.lock");
        return ERROR;
    }

    return NULL;
}

static void
sy_module_link(sy_module_entry_t_t *current, sy_module_entry_t_t *it)
{
    SyModule_t *module = sy_module_get();

    it->next = current;
    if (current)
    {
        it->previous = current->previous;
        current->previous = it;
        if (current->previous)
        {
            current->previous->next = it;
        }
    }
    
    if(module->begin == current)
    {
        module->begin = it;
    }
}

static sy_module_entry_t_t *
sy_module_push(const char *path, sy_node_t *root)
{
    SyModule_t *module = sy_module_get();

    if (sy_mutex_lock(&module->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "SyModule.lock");
        return ERROR;
    }

    for (sy_module_entry_t_t *item = module->begin; item != NULL; item = item->next)
    {
        if (strcmp(item->path, path) == 0)
        {
            if (sy_mutex_unlock(&module->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "SyModule.lock");
                return ERROR;
            }
            return NULL;
        }
    }

    sy_module_entry_t_t *entry = (sy_module_entry_t_t *)sy_memory_calloc(1, sizeof(sy_module_entry_t_t));
    if (!entry)
    {
        if (sy_mutex_unlock(&module->lock) < 0)
        {
            sy_error_system("'%s' could not unlock", "SyModule.lock");
            return ERROR;
        }

        sy_error_no_memory();
        return ERROR;
    }

    strcpy(entry->path, path);
    entry->root = root;

    sy_module_link(module->begin, entry);

    if (sy_mutex_unlock(&module->lock) < 0)
    {
        sy_memory_free(entry);
        sy_error_system("'%s' could not unlock", "SyModule.lock");
        return ERROR;
    }

    return entry;
}

typedef struct {
    /* inputs */
    char *base_file;
    /* outputs */
    sy_module_entry_t_t *module;
} thread_data_t;

#if _WIN32
static DWORD WINAPI 
sy_module_load_by_thread(LPVOID arg)
#else
static void * 
sy_module_load_by_thread(void *arg)
#endif
{
    assert (arg != NULL);

    thread_data_t *data = (thread_data_t*)arg;
    data->module = ERROR;

    sy_syntax_t *syntax = sy_syntax_create(data->base_file);
    if(!syntax)
    {
        goto region_finalize;
    }

    sy_node_t *root = sy_syntax_module(syntax);
    if(!root)
    {
        goto region_finalize;
    }

    sy_module_entry_t_t *module = sy_module_push(data->base_file, root);
    if (module == ERROR)
    {
        goto region_finalize;
    }
    else
    if (module == NULL)
    {
        module = sy_module_get_entry_by_path(data->base_file);
        if (module == ERROR)
        {
            goto region_finalize;
        }

        data->module = module;
    }
    else
    {
        module = sy_module_get_entry_by_path(data->base_file);
        if (module == ERROR)
        {
            goto region_finalize;
        }
        
        int32_t r4 = sy_semantic_module(module->root);
        if(r4 == -1)
        {
            goto region_finalize;
        }

        data->module = module;
    }

    region_finalize:
    if (sy_thread_join_all_childrens() < 0)
    {
        goto region_error;
    }

    if (sy_thread_exit() < 0)
    {
        goto region_error;
    }

    return data;

    region_error:
    data->module = ERROR;
    return data;
}

sy_module_entry_t_t *
sy_module_load(char *path)
{
	char base_file[MAX_PATH];

	if (SyPath_IsRoot(path))
	{
		char base_path[MAX_PATH];
		SyPath_Normalize(getenv (ENV_LIBRARY_KEY), base_path, MAX_PATH);
		SyPath_Join(base_path, path + 2, base_file, MAX_PATH);
	}
	else
	{
		char base_path[MAX_PATH];
		SyPath_GetCurrentDirectory(base_path, MAX_PATH);
		if(SyPath_IsRelative(path))
		{
			SyPath_Join(base_path, path, base_file, MAX_PATH);
		}
		else 
		{
			SyPath_Normalize(path, base_file, MAX_PATH);
		}
	}

	sy_module_entry_t_t *module_entry = sy_module_get_entry_by_path(base_file);
    if (module_entry == ERROR)
    {
        return ERROR;
    }
    else
    if (module_entry == NULL)
    {
        thread_data_t data;
        data.base_file = base_file;

        sy_thread_t *thread = sy_thread_create(sy_module_load_by_thread, &data);
        if (thread == ERROR)
        {
            sy_error_system("new thread not created\n");
            return ERROR;
        }

        if (sy_thread_join(thread) < 0)
        {
            return ERROR;
        }

        if (data.module == ERROR)
        {
            return ERROR;
        }

        module_entry = data.module;
    }

	return module_entry;
}

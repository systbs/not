#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "../../types/types.h"
#include "../../container/list.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../program.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../syntax/syntax.h"
#include "../error.h"
#include "semantic.h"

int32_t
semantic_subset(program_t *program, node_t *n1, node_t *n2)
{
    if (n1->id == n2->id)
    {
        return 1;
    }

    if (n2->kind == NODE_KIND_CLASS)
    {
        node_class_t *class2 = (node_class_t *)n2->value;
        if (class2->heritages != NULL)
        {
            node_t *node2 = class2->heritages;
            node_block_t *block2 = (node_block_t *)node2->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_HERITAGE)
                {
                    node_heritage_t *heritage2 = (node_heritage_t *)item2->value;

                    if (heritage2->value_update != NULL)
                    {
                        node_t *node3 = heritage2->value_update;
                        int32_t r3 = semantic_subset(program, n1, node3);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r3 == 1)
                        {
                            return 1;
                        }
                    }
                    else
                    {
                        node_t *node3 = heritage2->type;

                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r2 = semantic_resolve(program, node3->parent, node3, response2, SEMANTIC_FLAG_NONE);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r2 == 1)
                        {
                            uint64_t cnt_response2 = 0;

                            ilist_t *b2;
                            for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                            {
                                cnt_response2 += 1;

                                node_t *item3 = (node_t *)b2->value;
                                int32_t r3 = semantic_subset(program, n1, item3);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 1)
                                {
                                    list_destroy(response2);
                                    return 1;
                                }
                            }

                            if (cnt_response2 == 0)
                            {
                                semantic_error(program, node3, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        list_destroy(response2);
                    }
                }
            }
        }
    }

    return 0;
}

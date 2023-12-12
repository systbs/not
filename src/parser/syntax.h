#pragma once
#ifndef __SYNTAX_H__

enum syntax_type_of_flag {
  SYNTAX_TYPE_OF_NONE,
  SYNTAX_TYPE_OF_REFRENCE
};

int32_t
syntax_run(graph_t *graph);

#endif
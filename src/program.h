#pragma once
#ifndef __PROGRAM_H__

typedef struct program {
    char base_path[_MAX_PATH];
    char base_file[_MAX_PATH + _MAX_FNAME + _MAX_EXT];
} program_t;

#endif
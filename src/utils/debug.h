#pragma once
#ifndef __DEBUG_H__
#define __DEBUG_H__

#define PRINT_LINE(...) printf(__VA_ARGS__);printf("\n");fflush(stdout)

#ifdef DEBUG
#define DEBUG_FORMAT(...) PRINT_LINE(__VA_ARGS__)
#else 
#define DEBUG_FORMAT(...)
#endif

#endif //__DEBUG_H__

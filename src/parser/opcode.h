#pragma once
#ifndef __OPCODE_H__
#define __OPCODE_H__ 1

typedef enum opcode_type
{
    OPCODE_NOP = 0,
	OPCODE_ENT,
    OPCODE_LEV,
    OPCODE_IMM,
    OPCODE_PUSH,
    OPCODE_POP,
    OPCODE_JMP,
    OPCODE_JZ,
    OPCODE_PSAVE,
    OPCODE_LS,     // LAST SELECT
    OPCODE_CATCH,
    OPCODE_TENT,
    OPCODE_TLEV,
    OPCODE_LABEL,
    OPCODE_PACK,
    OPCODE_THROW,
    OPCODE_RET,
} opcode_type_t;

#endif
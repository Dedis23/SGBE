#ifndef __GB_CPU_H
#define __GB_CPU_H

#include "gb_registers.h"

class GBCPU
{
public:

private:
	ByteRegister A, B, C, D, E, F, H, L; // the regular 8 bit-registers
	WordRegister SP, PC; // 16-bit registers, stack pointer and program counter
	FlagRegister flag;
};

#endif
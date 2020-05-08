#ifndef __GB_CPU_H
#define __GB_CPU_H

#include "gb_registers.h"

class GBCPU
{
public:
	GBCPU();
	virtual ~GBCPU() = default;
	GBCPU(const GBCPU&) = delete;
	GBCPU& operator=(const GBCPU&) = delete;

private:
	/* registers */
	ByteRegister A, B, C, D, E, F, H, L; // the regular 8 bit-registers
	WordRegister SP, PC; // 16-bit registers, stack pointer and program counter
	FlagRegister Flag; // flag register, high nible is affected: Z N H C 0 0 0 0
	Pair8BRegisters AF, BC, DE, HL; // 2x8-bit registers paired together
};

#endif
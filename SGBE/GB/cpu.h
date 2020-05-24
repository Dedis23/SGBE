/************************************************
 *			Copyright (C) 2020 Dedi Sidi		*
 *												*
 *			The CPU of the gameboy				*
 ************************************************/

#ifndef __CPU_H
#define __CPU_H

#include "registers.h"
#include <unordered_map>

class CPU
{
public:
	CPU();
	virtual ~CPU() = default;
	CPU(const CPU&) = delete;
	CPU& operator=(const CPU&) = delete;

	void Step();
	void Reset();

private:
	/* registers */
	ByteRegister A, B, C, D, E, F, H, L; // 8 bit-registers, accumulator and general purpose ones
	WordRegister SP, PC; // 16-bit registers, stack pointer and program counter
	FlagRegister Flag; // flag register, high nible is affected: Z N H C 0 0 0 0
	Pair8BRegisters AF, BC, DE, HL; // 2x8-bit registers paired together
	bool m_IME; // Interrupt master enable

private:
	/* opcodes */
	//std::unordered_map<byte, >
	//struct OPCode
	//{
	//	void operation(void) {}
	//	std::string name;
	//	uint8_t		cycles;
	//};
};

#endif
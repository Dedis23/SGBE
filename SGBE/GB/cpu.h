/************************************************
 *			Copyright (C) 2020 Dedi Sidi		*
 *												*
 *			The CPU of the gameboy				*
 ************************************************/

#include <unordered_map>
#include "utility.h"
#include "mmu.h"

#ifndef __CPU_H
#define __CPU_H

class CPU
{
public:
	CPU(MMU& i_MMU);
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
	/* components */
	MMU& m_MMU;

private:
	/* opcodes */
	typedef uint32_t(CPU::* OPCodeFuntion)(byte operand);
	struct OPCode
	{
		OPCodeFuntion Operation;
		std::string Name;
	};

	static const std::vector<OPCode> m_OPCodeMap;
	uint32_t stam(byte operand);
};

#endif
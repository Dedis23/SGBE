/************************************************
 *			Copyright (C) 2020 Dedi Sidi		*
 *												*
 *			The CPU of the gameboy				*
 ************************************************/

/* Most of the information were take from gameboy cpu manual pdf: *
 * http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf				  */

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
	byte readNextByte();
	word readNextWord();

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
	/* OPCodes - struct and map */
	typedef void(CPU::* OPCodeFuntion)();
	struct OPCodeData
	{
		OPCodeFuntion Operation;
		std::string Name;
		uint32_t Cycles;
	};
	static const std::vector<OPCodeData> m_OPCodeDataMap;

	// CPU Operations
	void LD_nn_n(ByteRegister& i_Operand);

	// OPCode Functions
	void OPCode_06();
	void OPCode_0E();
	void OPCode_16();
	void OPCode_1E();
	void OPCode_26();
	void OPCode_2E();
};

#endif
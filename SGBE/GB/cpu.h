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

	/* CPU Operations */
	void LD_nn_n(IRegister& i_DestRegister);
	void LD_nn_nn(IRegister& i_DestRegister);
	void LD_r1_r2(IRegister& i_DestRegister, const IRegister& i_SrcRegister);
	void LD_r1_r2(IRegister& i_DestRegister, const WordAddress& i_SrcMemory);
	void LD_r1_r2(const WordAddress& i_DestMemory, const IRegister& i_SrcRegister);

	/* OPCode Functions */
	void OPCode_06();
	void OPCode_0E();
	void OPCode_16();
	void OPCode_1E();
	void OPCode_26();
	void OPCode_2E();

	void OPCode_7F();
	void OPCode_78();
	void OPCode_79();
	void OPCode_7A();
	void OPCode_7B();
	void OPCode_7C();
	void OPCode_7D();
	void OPCode_7E();

	void OPCode_40();
	void OPCode_41();
	void OPCode_42();
	void OPCode_43();
	void OPCode_44();
	void OPCode_45();
	void OPCode_46();

	void OPCode_48();
	void OPCode_49();
	void OPCode_4A();
	void OPCode_4B();
	void OPCode_4C();
	void OPCode_4D();
	void OPCode_4E();

	void OPCode_50();
	void OPCode_51();
	void OPCode_52();
	void OPCode_53();
	void OPCode_54();
	void OPCode_55();
	void OPCode_56();

	void OPCode_58();
	void OPCode_59();
	void OPCode_5A();
	void OPCode_5B();
	void OPCode_5C();
	void OPCode_5D();
	void OPCode_5E();

	void OPCode_60();
	void OPCode_61();
	void OPCode_62();
	void OPCode_63();
	void OPCode_64();
	void OPCode_65();
	void OPCode_66();

	void OPCode_68();
	void OPCode_69();
	void OPCode_6A();
	void OPCode_6B();
	void OPCode_6C();
	void OPCode_6D();
	void OPCode_6E();

	void OPCode_70();
	void OPCode_71();
	void OPCode_72();
	void OPCode_73();
	void OPCode_74();
	void OPCode_75();
	void OPCode_36();

	void OPCode_0A();
	void OPCode_1A();
	void OPCode_FA();
	void OPCode_3E();

	void OPCode_47();
	void OPCode_4F();
	void OPCode_57();
	void OPCode_5F();
	void OPCode_67();
	void OPCode_6F();
	void OPCode_02();
	void OPCode_12();
	void OPCode_77();
	void OPCode_EA();

	void OPCode_F2();
	void OPCode_E2();

	void OPCode_3A();
	void OPCode_32();

	void OPCode_2A();
	void OPCode_22();

	void OPCode_E0();
	void OPCode_F0();

	void OPCode_01();
	void OPCode_11();
	void OPCode_21();
	void OPCode_31();

	void OPCode_F9();
};

#endif
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
	enum class JumpConditions
	{
		NZ,
		Z,
		NC,
		C,
	};

	byte readNextByte();
	word readNextWord();
	sbyte readNextSignedByte();
	bool checkJumpCondition(JumpConditions i_Condition);

private:
	/* registers */
	ByteRegister A, B, C, D, E, F, H, L; // 8 bit-registers, accumulator and general purpose ones
	WordRegister SP, PC; // 16-bit registers, stack pointer and program counter
	FlagRegister Flag; // flag register, high nible is affected: Z N H C 0 0 0 0
	Pair8BRegisters AF, BC, DE, HL; // 2x8-bit registers paired together
	bool m_IME; // Interrupt master enable
	bool m_HALT; // is cpu halted boolean (wait until an interrupt occurs)
	bool m_IsCCJump;

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
		uint32_t ConditionalCycles; // some conditional jump instructions will require more cycles if the condition is met
	};
	static const std::vector<OPCodeData> m_OPCodeDataMap;
	static const std::vector<OPCodeData> m_CB_OPCodeDataMap;

	/* CPU Operations */
	void LD_nn_n(IRegister& i_DestRegister);
	void LD_n_nn(IRegister& i_DestRegister);
	void LD_r1_r2(IRegister& i_DestRegister, const IRegister& i_SrcRegister);
	void LD_r1_r2(IRegister& i_DestRegister, const WordAddress& i_SrcMemory);
	void LD_r1_r2(const WordAddress& i_DestMemory, const IRegister& i_SrcRegister);
	void LD_HL_SP_n();
	void PUSH(word i_Value);
	void POP(word& i_Value);
	void ADD(byte i_Value);
	void ADD_HL(word i_Value);
	void ADD_SP();
	void ADC(byte i_Value);
	void SUB(byte i_Value);
	void SBC(byte i_Value);
	void AND(byte i_Value);
	void OR(byte i_Value);
	void XOR(byte i_Value);
	void CP(byte i_Value);
	void INC(IRegister& i_DestRegister);
	void INC_no_flags(IRegister& i_DestRegister);
	void DEC(IRegister& i_DestRegister);
	void DEC_no_flags(IRegister& i_DestRegister);
	void DAA();
	void CPL();
	void CCF();
	void SCF();
	void _NOP();
	void HALT();
	void STOP();
	void DI();
	void EI();
	void JP_nn();
	void JP_cc_nn(JumpConditions i_Condition);
	void JP_hl();
	void JR_n();
	void JR_cc_n(JumpConditions i_Condition);
	void CALL_nn();
	void CALL_cc_nn(JumpConditions i_Condition);
	void RST_n(word i_Value);
	void RET();
	void RET_cc(JumpConditions i_Condition);
	void RETI();
	void SWAP(IRegister& i_DestRegister);
	void RLC_n(IRegister& i_DestRegister);
	void RL_n(IRegister& i_DestRegister);
	void RRC_n(IRegister& i_DestRegister);
	void RR_n(IRegister& i_DestRegister);
	void SLA_n(IRegister& i_DestRegister);
	void SRA_n(IRegister& i_DestRegister);
	void SRL_n(IRegister& i_DestRegister);
	void BIT_b_r(byte i_BitNumber, byte i_Value);
	void SET_b_r(byte i_BitNumber, byte& o_Value);

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

	void OPCode_F8();

	void OPCode_08();

	void OPCode_F5();
	void OPCode_C5();
	void OPCode_D5();
	void OPCode_E5();

	void OPCode_F1();
	void OPCode_C1();
	void OPCode_D1();
	void OPCode_E1();

	void OPCode_87();
	void OPCode_80();
	void OPCode_81();
	void OPCode_82();
	void OPCode_83();
	void OPCode_84();
	void OPCode_85();
	void OPCode_86();
	void OPCode_C6();

	void OPCode_8F();
	void OPCode_88();
	void OPCode_89();
	void OPCode_8A();
	void OPCode_8B();
	void OPCode_8C();
	void OPCode_8D();
	void OPCode_8E();
	void OPCode_CE();

	void OPCode_97();
	void OPCode_90();
	void OPCode_91();
	void OPCode_92();
	void OPCode_93();
	void OPCode_94();
	void OPCode_95();
	void OPCode_96();
	void OPCode_D6();

	void OPCode_9F();
	void OPCode_98();
	void OPCode_99();
	void OPCode_9A();
	void OPCode_9B();
	void OPCode_9C();
	void OPCode_9D();
	void OPCode_9E();
	void OPCode_DE();

	void OPCode_A7();
	void OPCode_A0();
	void OPCode_A1();
	void OPCode_A2();
	void OPCode_A3();
	void OPCode_A4();
	void OPCode_A5();
	void OPCode_A6();
	void OPCode_E6();

	void OPCode_B7();
	void OPCode_B0();
	void OPCode_B1();
	void OPCode_B2();
	void OPCode_B3();
	void OPCode_B4();
	void OPCode_B5();
	void OPCode_B6();
	void OPCode_F6();

	void OPCode_AF();
	void OPCode_A8();
	void OPCode_A9();
	void OPCode_AA();
	void OPCode_AB();
	void OPCode_AC();
	void OPCode_AD();
	void OPCode_AE();
	void OPCode_EE();

	void OPCode_BF();
	void OPCode_B8();
	void OPCode_B9();
	void OPCode_BA();
	void OPCode_BB();
	void OPCode_BC();
	void OPCode_BD();
	void OPCode_BE();
	void OPCode_FE();

	void OPCode_3C();
	void OPCode_04();
	void OPCode_0C();
	void OPCode_14();
	void OPCode_1C();
	void OPCode_24();
	void OPCode_2C();
	void OPCode_34();

	void OPCode_3D();
	void OPCode_05();
	void OPCode_0D();
	void OPCode_15();
	void OPCode_1D();
	void OPCode_25();
	void OPCode_2D();
	void OPCode_35();

	void OPCode_09();
	void OPCode_19();
	void OPCode_29();
	void OPCode_39();

	void OPCode_E8();

	void OPCode_03();
	void OPCode_13();
	void OPCode_23();
	void OPCode_33();

	void OPCode_0B();
	void OPCode_1B();
	void OPCode_2B();
	void OPCode_3B();

	void OPCode_27();

	void OPCode_2F();

	void OPCode_3F();

	void OPCode_37();

	void OPCode_00();

	void OPCode_76();

	void OPCode_10();

	void OPCode_F3();
				
	void OPCode_FB();

	void OPCode_07();

	void OPCode_17();

	void OPCode_0F();

	void OPCode_1F();

	void OPCode_C3();

	void OPCode_C2();
	void OPCode_CA();
	void OPCode_D2();
	void OPCode_DA();

	void OPCode_E9();

	void OPCode_18();

	void OPCode_20();
	void OPCode_28();
	void OPCode_30();
	void OPCode_38();

	void OPCode_CD();

	void OPCode_C4();
	void OPCode_CC();
	void OPCode_D4();
	void OPCode_DC();

	void OPCode_C7();
	void OPCode_CF();
	void OPCode_D7();
	void OPCode_DF();
	void OPCode_E7();
	void OPCode_EF();
	void OPCode_F7();
	void OPCode_FF();

	void OPCode_C9();

	void OPCode_C0();
	void OPCode_C8();
	void OPCode_D0();
	void OPCode_D8();

	void OPCode_D9();

	void OPCode_CB_37();
	void OPCode_CB_30();
	void OPCode_CB_31();
	void OPCode_CB_32();
	void OPCode_CB_33();
	void OPCode_CB_34();
	void OPCode_CB_35();
	void OPCode_CB_36();

	void OPCode_CB_07();
	void OPCode_CB_00();
	void OPCode_CB_01();
	void OPCode_CB_02();
	void OPCode_CB_03();
	void OPCode_CB_04();
	void OPCode_CB_05();
	void OPCode_CB_06();

	void OPCode_CB_17();
	void OPCode_CB_10();
	void OPCode_CB_11();
	void OPCode_CB_12();
	void OPCode_CB_13();
	void OPCode_CB_14();
	void OPCode_CB_15();
	void OPCode_CB_16();

	void OPCode_CB_0F();
	void OPCode_CB_08();
	void OPCode_CB_09();
	void OPCode_CB_0A();
	void OPCode_CB_0B();
	void OPCode_CB_0C();
	void OPCode_CB_0D();
	void OPCode_CB_0E();

	void OPCode_CB_1F();
	void OPCode_CB_18();
	void OPCode_CB_19();
	void OPCode_CB_1A();
	void OPCode_CB_1B();
	void OPCode_CB_1C();
	void OPCode_CB_1D();
	void OPCode_CB_1E();

	void OPCode_CB_27();
	void OPCode_CB_20();
	void OPCode_CB_21();
	void OPCode_CB_22();
	void OPCode_CB_23();
	void OPCode_CB_24();
	void OPCode_CB_25();
	void OPCode_CB_26();

	void OPCode_CB_2F();
	void OPCode_CB_28();
	void OPCode_CB_29();
	void OPCode_CB_2A();
	void OPCode_CB_2B();
	void OPCode_CB_2C();
	void OPCode_CB_2D();
	void OPCode_CB_2E();

	void OPCode_CB_3F();
	void OPCode_CB_38();
	void OPCode_CB_39();
	void OPCode_CB_3A();
	void OPCode_CB_3B();
	void OPCode_CB_3C();
	void OPCode_CB_3D();
	void OPCode_CB_3E();

	void OPCode_CB_47();
	void OPCode_CB_40();
	void OPCode_CB_41();
	void OPCode_CB_42();
	void OPCode_CB_43();
	void OPCode_CB_44();
	void OPCode_CB_45();
	void OPCode_CB_46();

	void OPCode_CB_4F();
	void OPCode_CB_48();
	void OPCode_CB_49();
	void OPCode_CB_4A();
	void OPCode_CB_4B();
	void OPCode_CB_4C();
	void OPCode_CB_4D();
	void OPCode_CB_4E();

	void OPCode_CB_57();
	void OPCode_CB_50();
	void OPCode_CB_51();
	void OPCode_CB_52();
	void OPCode_CB_53();
	void OPCode_CB_54();
	void OPCode_CB_55();
	void OPCode_CB_56();

	void OPCode_CB_5F();
	void OPCode_CB_58();
	void OPCode_CB_59();
	void OPCode_CB_5A();
	void OPCode_CB_5B();
	void OPCode_CB_5C();
	void OPCode_CB_5D();
	void OPCode_CB_5E();

	void OPCode_CB_67();
	void OPCode_CB_60();
	void OPCode_CB_61();
	void OPCode_CB_62();
	void OPCode_CB_63();
	void OPCode_CB_64();
	void OPCode_CB_65();
	void OPCode_CB_66();

	void OPCode_CB_6F();
	void OPCode_CB_68();
	void OPCode_CB_69();
	void OPCode_CB_6A();
	void OPCode_CB_6B();
	void OPCode_CB_6C();
	void OPCode_CB_6D();
	void OPCode_CB_6E();

	void OPCode_CB_77();
	void OPCode_CB_70();
	void OPCode_CB_71();
	void OPCode_CB_72();
	void OPCode_CB_73();
	void OPCode_CB_74();
	void OPCode_CB_75();
	void OPCode_CB_76();

	void OPCode_CB_7F();
	void OPCode_CB_78();
	void OPCode_CB_79();
	void OPCode_CB_7A();
	void OPCode_CB_7B();
	void OPCode_CB_7C();
	void OPCode_CB_7D();
	void OPCode_CB_7E();

	void OPCode_CB_C7();
	void OPCode_CB_C0();
	void OPCode_CB_C1();
	void OPCode_CB_C2();
	void OPCode_CB_C3();
	void OPCode_CB_C4();
	void OPCode_CB_C5();
	void OPCode_CB_C6();

	void OPCode_CB_CF();
	void OPCode_CB_C8();
	void OPCode_CB_C9();
	void OPCode_CB_CA();
	void OPCode_CB_CB();
	void OPCode_CB_CC();
	void OPCode_CB_CD();
	void OPCode_CB_CE();

	void OPCode_CB_D7();
	void OPCode_CB_D0();
	void OPCode_CB_D1();
	void OPCode_CB_D2();
	void OPCode_CB_D3();
	void OPCode_CB_D4();
	void OPCode_CB_D5();
	void OPCode_CB_D6();

	void OPCode_CB_DF();
	void OPCode_CB_D8();
	void OPCode_CB_D9();
	void OPCode_CB_DA();
	void OPCode_CB_DB();
	void OPCode_CB_DC();
	void OPCode_CB_DD();
	void OPCode_CB_DE();
};

#endif
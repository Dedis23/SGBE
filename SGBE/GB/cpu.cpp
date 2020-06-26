#include "cpu.h"

CPU::CPU(MMU& i_MMU) : m_MMU(i_MMU), AF(A, F), BC(B, C), DE(D, E), HL(H, L), m_IME(false) {}

void CPU::Step()
{
	// read next instruction opcode
	byte OPCode = readNextByte();
	OPCodeData OPCodeData = m_OPCodeDataMap[OPCode];
	LOG_INFO(true, NOP, "Executing " << OPCodeData.Name << " in address 0x" << PC.GetValue() - 1);
	OPCodeData.Operation;
}

void CPU::Reset()
{
	A.SetValue(0);
	B.SetValue(0);
	C.SetValue(0);
	D.SetValue(0);
	E.SetValue(0);
	F.SetValue(0);
	H.SetValue(0);
	L.SetValue(0);
	SP.SetValue(0);
	PC.SetValue(0);
	Flag.SetValue(0);
	m_IME = false;
}

byte CPU::readNextByte()
{
	byte nextByte = m_MMU.Read(PC.GetValue());
	PC.Increment();
	return nextByte;
}

word CPU::readNextWord()
{
	byte lowByte = readNextByte();
	byte highByte = readNextByte();
	return static_cast<word>(highByte << 8 | lowByte);
}

sbyte CPU::readNextSignedByte()
{
	sbyte nextSbyte = m_MMU.Read(PC.GetValue());
	PC.Increment();
	return nextSbyte;
}

/* OPCodes */

/*
	Operation:
	LD nn, n

	Description:
	Put value n (byte from memory) into nn (register).
*/
void CPU::LD_nn_n(IRegister& i_DestRegister)
{
	byte n = readNextByte();
	i_DestRegister.SetValue(n);
}

/*
	Operation:
	LD n, nn

	Description:
	Put value nn (word from memory) into n (16 bit register).
*/
void CPU::LD_n_nn(IRegister& i_DestRegister)
{
	word nn = readNextWord();
	i_DestRegister.SetValue(nn);
}

/*
	Operation:
	LD r1, r2

	Description:
	Put value r2 into r1.

*/
void CPU::LD_r1_r2(IRegister& i_DestRegister, const IRegister& i_SrcRegister)
{
	word val = i_SrcRegister.GetValue();
	i_DestRegister.SetValue(val);
}

void CPU::LD_r1_r2(IRegister& i_DestRegister, const WordAddress& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	i_DestRegister.SetValue(val);
}

void CPU::LD_r1_r2(const WordAddress& i_DestMemory, const IRegister& i_SrcRegister)
{
	byte val = static_cast<byte>(i_SrcRegister.GetValue());
	m_MMU.Write(i_DestMemory, val);
}

/*
	Operation:
	LDHL SP,n

	Description:
	Put SP + n effective address into HL
	n = one byte signed immediate value
	Z - Reset
	N - Reset
	H - Set or reset according to operation
	C - Set or reset according to operation
*/
void CPU::LD_HL_SP_n()
{
	sbyte n = readNextSignedByte();
	word res = SP.GetValue() + n;

	word flagCheck = SP.GetValue() ^ n ^ (SP.GetValue() + n);
	Flag.SetZ(false);
	Flag.SetN(false);
	(flagCheck & 0x10) == 0x10 ? Flag.SetH(true) : Flag.SetH(false);
	(flagCheck & 0x100) == 0x100 ? Flag.SetC(true) : Flag.SetC(false);

	HL.SetValue(res);
}

/*
	Operation:
	PUSH nn

	Description:
	Push register pair nn onto stack
	Decrement Stack Pointer (SP) twice
*/
void CPU::PUSH(Pair8BRegisters& i_RegisterPair)
{
	SP.Decrement();
	word addr = SP.GetValue();
	byte highVal = static_cast<byte>(i_RegisterPair.GetHighRegister().GetValue());
	m_MMU.Write(addr, highVal);

	SP.Decrement();
	addr = SP.GetValue();
	byte lowVal = static_cast<byte>(i_RegisterPair.GetLowRegister().GetValue());
	m_MMU.Write(addr, lowVal);
}

/*
	Operation:
	POP nn

	Description:
	Pop two bytes off stack into register pair nn
	Increment Stack Pointer (SP) twice
*/
void CPU::POP(Pair8BRegisters& i_RegisterPair)
{
	word addr = SP.GetValue();
	byte lowVal = m_MMU.Read(addr);
	i_RegisterPair.GetLowRegister().SetValue(lowVal);
	SP.Increment();

	addr = SP.GetValue();
	byte highVal = m_MMU.Read(addr);
	i_RegisterPair.GetHighRegister().SetValue(highVal);
	SP.Increment();
}

/*
	Operation:
	ADD A, n

	Description:
	Add n to A
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Set if carry from bit 7
*/
void CPU::ADD(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal + i_Value;
	A.SetValue(res);

	res == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(false);
	((aVal & 0xF) + (i_Value & 0xF)) > 0xF ? Flag.SetH(true) : Flag.SetH(false);
	((aVal & 0xFF) + (i_Value & 0xFF)) > 0xFF ? Flag.SetC(true) : Flag.SetC(false);
}

/*
	Operation:
	ADD HL, n

	Description:
	Add n to HL
	Z - Not affected
	N - Reset
	H - Set if carry from bit 11
	C - Set if carry from bit 15
*/
void CPU::ADD_HL(word i_Value)
{
	word hlVal = HL.GetValue();
	word res = hlVal + i_Value;
	HL.SetValue(res);

	Flag.SetN(false);
	((hlVal & 0xFFF) + (i_Value & 0xFFF)) > 0xFFF ? Flag.SetH(true) : Flag.SetH(false);
	((hlVal & 0xFFFF) + (i_Value & 0xFFFF)) > 0xFFFF ? Flag.SetC(true) : Flag.SetC(false);
}

/*
	Operation:
	ADD SP, n

	Description:
	Add n to SP
	n = one byte signed immediate value (#).
	Z - Reset
	N - Reset
	H - Set or reset according to operation
	C - Set or reset according to operation
*/
void CPU::ADD_SP()
{
	sbyte val = readNextSignedByte();
	word spVal = SP.GetValue();
	int res = static_cast<int>(spVal + val);

	Flag.SetZ(false);
	Flag.SetN(false);
	if (val >= 0)
	{
		((spVal & 0xF) + (val & 0xF)) > 0xF ? Flag.SetH(true) : Flag.SetH(false);
		((spVal & 0xFF) + (val)) > 0xFF ? Flag.SetC(true) : Flag.SetC(false);
	}
	else
	{
		(res & 0xF) <= (spVal & 0xF) ? Flag.SetH(true) : Flag.SetH(false);
		(res & 0xFF) <= (spVal & 0xFF) ? Flag.SetC(true) : Flag.SetC(false);
	}
}

/*
	Operation:
	ADC A, n

	Description:
	Add n + Carry flag to A
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Set if carry from bit 7
*/
void CPU::ADC(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte carry = Flag.GetC();
	byte res = aVal + i_Value + carry;
	A.SetValue(res);

	res == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(false);
	((aVal & 0xF) + (i_Value & 0xF) + carry) > 0xF ? Flag.SetH(true) : Flag.SetH(false);
	((aVal & 0xFF) + (i_Value & 0xFF) + carry) > 0xFF ? Flag.SetC(true) : Flag.SetC(false);
}

/*
	Operation:
	SUB n

	Description:
	Subtract n to A
	Z - Set if result is zero
	N - Set
	H - Set if no borrow from bit 4
	C - Set if no borrow
*/
void CPU::SUB(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal - i_Value;
	A.SetValue(res);

	res == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(true);
	((aVal & 0xF) - (i_Value & 0xF)) < 0x0 ? Flag.SetH(true) : Flag.SetH(false);
	aVal < i_Value ? Flag.SetC(true) : Flag.SetC(false);
}

/*
	Operation:
	SBC A, n

	Description:
	Subtract n + Carry flag from A
	Z - Set if result is zero
	N - Set
	H - Set if no borrow from bit 4
	C - Set if no borrow
*/
void CPU::SBC(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte carry = Flag.GetC();
	byte res = aVal - carry - i_Value;
	A.SetValue(res);

	res == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(true);
	((aVal & 0xF) - (i_Value & 0xF) - carry) < 0x0 ? Flag.SetH(true) : Flag.SetH(false);
	aVal < i_Value + carry ? Flag.SetC(true) : Flag.SetC(false);
}

/*
	Operation:
	AND n

	Description:
	Logically AND n with A, result in A
	Z - Set if result is zero
	N - Reset
	H - Set
	C - Reset
*/
void CPU::AND(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal & i_Value;
	A.SetValue(res);

	res == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(false);
	Flag.SetH(true);
	Flag.SetC(false);
}

/*
	Operation:
	OR n

	Description:
	Logical OR n with A, result in A
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Reset
*/
void CPU::OR(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal | i_Value;
	A.SetValue(res);

	res == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(false);
	Flag.SetH(false);
	Flag.SetC(false);
}

/*
	Operation:
	XOR n

	Description:
	Logical exclusive OR n with register A, result in A
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Reset
*/
void CPU::XOR(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal ^ i_Value;
	A.SetValue(res);

	res == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(false);
	Flag.SetH(false);
	Flag.SetC(false);
}

/*
	Operation:
	CP n

	Description:
	Compare A with n. This is basically an A - n
	subtraction instruction but the results are thrown away
	Z - Set if result is zero
	N - Set
	H - Set if no borrow from bit 4
	C - Set if no borrow
*/
void CPU::CP(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal - i_Value;

	res == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(true);
	((aVal & 0xF) - (i_Value & 0xF)) < 0x0 ? Flag.SetH(true) : Flag.SetH(false);
	aVal < i_Value ? Flag.SetC(true) : Flag.SetC(false);
}

/*
	Operation:
	INC n

	Description:
	Increment register n
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Not affected
*/
void CPU::INC(IRegister& i_DestRegister)
{
	i_DestRegister.Increment();
	word regVal = i_DestRegister.GetValue();

	regVal == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(false);
	(regVal & 0xF) == 0x0 ? Flag.SetH(true) : Flag.SetH(false);
}

/*
	Operation:
	INC nn

	Description:
	Increment register nn
	nn = BC, DE, HL, SP
	No flags affected
*/
void CPU::INC_no_flags(IRegister& i_DestRegister)
{
	i_DestRegister.Increment();
}

/*
	Operation:
	DEC n

	Description:
	Decrement register n
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Not affected
*/
void CPU::DEC(IRegister& i_DestRegister)
{
	i_DestRegister.Decrement();
	word regVal = i_DestRegister.GetValue();

	regVal == 0x0 ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetN(false);
	(regVal & 0xF) == 0x0 ? Flag.SetH(true) : Flag.SetH(false);
}

/*
	Operation:
	DEC nn

	Description:
	Decrement register nn
	nn = BC, DE, HL, SP
	No flags affected
*/
void CPU::DEC_no_flags(IRegister& i_DestRegister)
{
	i_DestRegister.Decrement();
}

/*
	Operation:
	DAA

	Description:
	Decimal adjust register A.
	This instruction adjusts register A so that the
	correct representation of Binary Coded Decimal (BCD)
	is obtained.
	Z - Set if register A is zero.
	N - Not affected.
	H - Reset.
	C - Set or reset according to operation.

	a youtube video explaining the operation:
	https://www.youtube.com/watch?v=tvGWNLY3BOE
*/
void CPU::DAA()
{
	byte aVal = static_cast<byte>(A.GetValue());
	int correctionVal = 0x0;

	if (Flag.GetN())
	{
		if (Flag.GetH())
		{
			correctionVal -= 0x06;
		}

		if (Flag.GetC())
		{
			correctionVal -= 0x60;
		}
	}
	else
	{
		// if high nibble is greater than 9 or carry is set, add 0x60 to correction
		if ((((aVal & 0xF0) >> 4) > 0x9) || Flag.GetC())
		{
			correctionVal += 0x60;
		}

		// if low nibble is greater than 9 or half carry is set, add 0x6 to correction
		if (((aVal & 0xF) > 0x9) || Flag.GetH())
		{
			correctionVal += 0x6;
		}
	}

	int res = aVal + correctionVal;
	
	(res == 0x0) ? Flag.SetZ(true) : Flag.SetZ(false);
	Flag.SetH(false);
	((aVal & 0xFF) + (correctionVal & 0xFF)) > 0xFF ? Flag.SetC(true) : Flag.SetC(false);

	A.SetValue(static_cast<word>(res));
}

const std::vector<CPU::OPCodeData> CPU::m_OPCodeDataMap
{
	{ &OPCode_06, "LD B, n", 8 },
	{ &OPCode_0E, "LD C, n", 8 },
	{ &OPCode_16, "LD D, n", 8 },
	{ &OPCode_1E, "LD E, n", 8 },
	{ &OPCode_26, "LD H, n", 8 },
	{ &OPCode_2E, "LD L, n", 8 },

	{ &OPCode_7F, "LD A, A", 4 },
	{ &OPCode_78, "LD A, B", 4 },
	{ &OPCode_79, "LD A, C", 4 },
	{ &OPCode_7A, "LD A, D", 4 },
	{ &OPCode_7B, "LD A, E", 4 },
	{ &OPCode_7C, "LD A, H", 4 },
	{ &OPCode_7D, "LD A, L", 4 },
	{ &OPCode_7E, "LD A, (HL)", 8 },

	{ &OPCode_40, "LD B, B", 4 },
	{ &OPCode_41, "LD B, C", 4 },
	{ &OPCode_42, "LD B, D", 4 },
	{ &OPCode_43, "LD B, E", 4 },
	{ &OPCode_44, "LD B, H", 4 },
	{ &OPCode_45, "LD B, L", 4 },
	{ &OPCode_46, "LD B, (HL)", 8 },

	{ &OPCode_48, "LD C, B", 4 },
	{ &OPCode_49, "LD C, C", 4 },
	{ &OPCode_4A, "LD C, D", 4 },
	{ &OPCode_4B, "LD C, E", 4 },
	{ &OPCode_4C, "LD C, H", 4 },
	{ &OPCode_4D, "LD C, L", 4 },
	{ &OPCode_4E, "LD C, (HL)", 8 },

	{ &OPCode_50, "LD D, B", 4 },
	{ &OPCode_51, "LD D, C", 4 },
	{ &OPCode_52, "LD D, D", 4 },
	{ &OPCode_53, "LD D, E", 4 },
	{ &OPCode_54, "LD D, H", 4 },
	{ &OPCode_55, "LD D, L", 4 },
	{ &OPCode_56, "LD D, (HL)", 8 },

	{ &OPCode_58, "LD E, B", 4 },
	{ &OPCode_59, "LD E, C", 4 },
	{ &OPCode_5A, "LD E, D", 4 },
	{ &OPCode_5B, "LD E, E", 4 },
	{ &OPCode_5C, "LD E, H", 4 },
	{ &OPCode_5D, "LD E, L", 4 },
	{ &OPCode_5E, "LD E, (HL)", 8 },

	{ &OPCode_60, "LD H, B", 4 },
	{ &OPCode_61, "LD H, C", 4 },
	{ &OPCode_62, "LD H, D", 4 },
	{ &OPCode_63, "LD H, E", 4 },
	{ &OPCode_64, "LD H, H", 4 },
	{ &OPCode_65, "LD H, L", 4 },
	{ &OPCode_66, "LD H, (HL)", 8 },

	{ &OPCode_68, "LD L, B", 4 },
	{ &OPCode_69, "LD L, C", 4 },
	{ &OPCode_6A, "LD L, D", 4 },
	{ &OPCode_6B, "LD L, E", 4 },
	{ &OPCode_6C, "LD L, H", 4 },
	{ &OPCode_6D, "LD L, L", 4 },
	{ &OPCode_6E, "LD L, (HL)", 8 },

	{ &OPCode_70, "LD (HL), B", 8 },
	{ &OPCode_71, "LD (HL), C", 8 },
	{ &OPCode_72, "LD (HL), D", 8 },
	{ &OPCode_73, "LD (HL), E", 8 },
	{ &OPCode_74, "LD (HL), H", 8 },
	{ &OPCode_75, "LD (HL), L", 8 },
	{ &OPCode_36, "LD (HL), n", 12 },

	{ &OPCode_0A, "LD A, (BC)", 8 },
	{ &OPCode_1A, "LD A, (DE)", 8 },
	{ &OPCode_FA, "LD A, (nn)", 16 },
	{ &OPCode_3E, "LD A, n", 8 },

	{ &OPCode_47, "LD B, A", 4 },
	{ &OPCode_4F, "LD C, A", 4 },
	{ &OPCode_57, "LD D, A", 4 },
	{ &OPCode_5F, "LD E, A", 4 },
	{ &OPCode_67, "LD H, A", 4 },
	{ &OPCode_6F, "LD L, A", 4 },
	{ &OPCode_02, "LD (BC), A", 8 },
	{ &OPCode_12, "LD (DE), A", 8 },
	{ &OPCode_77, "LD (HL), A", 8 },
	{ &OPCode_EA, "LD (nn), A", 16 },

	{ &OPCode_F2, "LD A, (C)", 8 },
	{ &OPCode_E2, "LD (C), A", 8 },

	{ &OPCode_3A, "LDD A, (HL)", 8 },
	{ &OPCode_32, "LDD (HL), A", 8 },

	{ &OPCode_2A, "LDI A, (HL)", 8 },
	{ &OPCode_22, "LDI (HL), A", 8 },

	{ &OPCode_E0, "LDH (n), A", 12 },
	{ &OPCode_F0, "LDH A, (n)", 12 },

	{ &OPCode_01, "LD BC, nn", 12 },
	{ &OPCode_11, "LD DE, nn", 12 },
	{ &OPCode_21, "LD HL, nn", 12 },
	{ &OPCode_31, "LD SP, nn", 12 },

	{ &OPCode_F9, "LD SP, HL", 8 },

	{ &OPCode_F8, "LDHL SP, n", 12 },

	{ &OPCode_08, "LD (nn), SP", 20 },

	{ &OPCode_F5, "PUSH AF", 16 },
	{ &OPCode_C5, "PUSH BC", 16 },
	{ &OPCode_D5, "PUSH DE", 16 },
	{ &OPCode_E5, "PUSH HL", 16 },

	{ &OPCode_F1, "POP AF", 12 },
	{ &OPCode_C1, "POP BC", 12 },
	{ &OPCode_D1, "POP DE", 12 },
	{ &OPCode_E1, "POP HL", 12 },

	{ &OPCode_87, "ADD A, A", 4 },
	{ &OPCode_80, "ADD A, B", 4 },
	{ &OPCode_81, "ADD A, C", 4 },
	{ &OPCode_82, "ADD A, D", 4 },
	{ &OPCode_83, "ADD A, E", 4 },
	{ &OPCode_84, "ADD A, H", 4 },
	{ &OPCode_85, "ADD A, L", 4 },
	{ &OPCode_86, "ADD A, (HL)", 8 },
	{ &OPCode_C6, "ADD A, n", 8 },

	{ &OPCode_8F, "ADC A, A", 4 },
	{ &OPCode_88, "ADC A, B", 4 },
	{ &OPCode_89, "ADC A, C", 4 },
	{ &OPCode_8A, "ADC A, D", 4 },
	{ &OPCode_8B, "ADC A, E", 4 },
	{ &OPCode_8C, "ADC A, H", 4 },
	{ &OPCode_8D, "ADC A, L", 4 },
	{ &OPCode_8E, "ADC A, (HL)", 8 },
	{ &OPCode_CE, "ADC A, n", 8 },

	{ &OPCode_97, "SUB A", 4 },
	{ &OPCode_90, "SUB B", 4 },
	{ &OPCode_91, "SUB C", 4 },
	{ &OPCode_92, "SUB D", 4 },
	{ &OPCode_93, "SUB E", 4 },
	{ &OPCode_94, "SUB H", 4 },
	{ &OPCode_95, "SUB L", 4 },
	{ &OPCode_96, "SUB (HL)", 8 },
	{ &OPCode_D6, "SUB n", 8 },

	{ &OPCode_9F, "SBC A, A", 4 },
	{ &OPCode_98, "SBC A, B", 4 },
	{ &OPCode_99, "SBC A, C", 4 },
	{ &OPCode_9A, "SBC A, D", 4 },
	{ &OPCode_9B, "SBC A, E", 4 },
	{ &OPCode_9C, "SBC A, H", 4 },
	{ &OPCode_9D, "SBC A, L", 4 },
	{ &OPCode_9E, "SBC A, (HL)", 8 },
	{ &OPCode_DE, "SBC A, n", 8 },

	{ &OPCode_A7, "AND A", 4 },
	{ &OPCode_A0, "AND B", 4 },
	{ &OPCode_A1, "AND C", 4 },
	{ &OPCode_A2, "AND D", 4 },
	{ &OPCode_A3, "AND E", 4 },
	{ &OPCode_A4, "AND H", 4 },
	{ &OPCode_A5, "AND L", 4 },
	{ &OPCode_A6, "AND (HL)", 8 },
	{ &OPCode_E6, "AND n", 8 },

	{ &OPCode_B7, "OR A", 4 },
	{ &OPCode_B0, "OR B", 4 },
	{ &OPCode_B1, "OR C", 4 },
	{ &OPCode_B2, "OR D", 4 },
	{ &OPCode_B3, "OR E", 4 },
	{ &OPCode_B4, "OR H", 4 },
	{ &OPCode_B5, "OR L", 4 },
	{ &OPCode_B6, "OR (HL)", 8 },
	{ &OPCode_F6, "OR n", 8 },

	{ &OPCode_AF, "XOR A", 4 },
	{ &OPCode_A8, "XOR B", 4 },
	{ &OPCode_A9, "XOR C", 4 },
	{ &OPCode_AA, "XOR D", 4 },
	{ &OPCode_AB, "XOR E", 4 },
	{ &OPCode_AC, "XOR H", 4 },
	{ &OPCode_AD, "XOR L", 4 },
	{ &OPCode_AE, "XOR (HL)", 8 },
	{ &OPCode_EE, "XOR n", 8 },

	{ &OPCode_BF, "CP A", 4 },
	{ &OPCode_B8, "CP B", 4 },
	{ &OPCode_B9, "CP C", 4 },
	{ &OPCode_BA, "CP D", 4 },
	{ &OPCode_BB, "CP E", 4 },
	{ &OPCode_BC, "CP H", 4 },
	{ &OPCode_BD, "CP L", 4 },
	{ &OPCode_BE, "CP (HL)", 8 },
	{ &OPCode_FE, "CP n", 8 },

	{ &OPCode_3C, "INC A", 4 },
	{ &OPCode_04, "INC B", 4 },
	{ &OPCode_0C, "INC C", 4 },
	{ &OPCode_14, "INC D", 4 },
	{ &OPCode_1C, "INC E", 4 },
	{ &OPCode_24, "INC H", 4 },
	{ &OPCode_2C, "INC L", 4 },
	{ &OPCode_34, "INC (HL)", 12 },

	{ &OPCode_3D, "DEC A", 4 },
	{ &OPCode_05, "DEC B", 4 },
	{ &OPCode_0D, "DEC C", 4 },
	{ &OPCode_15, "DEC D", 4 },
	{ &OPCode_1D, "DEC E", 4 },
	{ &OPCode_25, "DEC H", 4 },
	{ &OPCode_2D, "DEC L", 4 },
	{ &OPCode_35, "DEC (HL)", 12 },

	{ &OPCode_09, "ADD HL, BC", 8 },
	{ &OPCode_19, "ADD HL, DE", 8 },
	{ &OPCode_29, "ADD HL, HL", 8 },
	{ &OPCode_39, "ADD HL, SP", 8 },

	{ &OPCode_E8, "ADD SP, #", 16 },

	{ &OPCode_03, "INC BC", 8 },
	{ &OPCode_13, "INC DE", 8 },
	{ &OPCode_23, "INC HL", 8 },
	{ &OPCode_33, "INC SP", 8 },

	{ &OPCode_0B, "DEC BC", 8 },
	{ &OPCode_1B, "DEC DE", 8 },
	{ &OPCode_2B, "DEC HL", 8 },
	{ &OPCode_3B, "DEC SP", 8 },

	{ &OPCode_27, "DAA", 4 },
};

void CPU::OPCode_06()
{
	LD_nn_n(B);
}

void CPU::OPCode_0E()
{
	LD_nn_n(C);
}

void CPU::OPCode_16()
{
	LD_nn_n(D);
}

void CPU::OPCode_1E()
{
	LD_nn_n(E);
}

void CPU::OPCode_26()
{
	LD_nn_n(H);
}

void CPU::OPCode_2E()
{
	LD_nn_n(L);
}

void CPU::OPCode_7F()
{
	LD_r1_r2(A, A);
}

void CPU::OPCode_78()
{
	LD_r1_r2(A, B);
}

void CPU::OPCode_79()
{
	LD_r1_r2(A, C);
}

void CPU::OPCode_7A()
{
	LD_r1_r2(A, D);
}

void CPU::OPCode_7B()
{
	LD_r1_r2(A, E);
}

void CPU::OPCode_7C()
{
	LD_r1_r2(A, H);
}

void CPU::OPCode_7D()
{
	LD_r1_r2(A, L);
}

void CPU::OPCode_7E()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_40()
{
	LD_r1_r2(B, B);
}

void CPU::OPCode_41()
{
	LD_r1_r2(B, C);
}

void CPU::OPCode_42()
{
	LD_r1_r2(B, D);
}

void CPU::OPCode_43()
{
	LD_r1_r2(B, E);
}

void CPU::OPCode_44()
{
	LD_r1_r2(B, H);
}

void CPU::OPCode_45()
{
	LD_r1_r2(B, L);
}

void CPU::OPCode_46()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(B, addr);
}

void CPU::OPCode_48()
{
	LD_r1_r2(C, B);
}

void CPU::OPCode_49()
{
	LD_r1_r2(C, C);
}

void CPU::OPCode_4A()
{
	LD_r1_r2(C, D);
}

void CPU::OPCode_4B()
{
	LD_r1_r2(C, E);
}

void CPU::OPCode_4C()
{
	LD_r1_r2(C, H);
}

void CPU::OPCode_4D()
{
	LD_r1_r2(C, L);
}

void CPU::OPCode_4E()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(C, addr);
}

void CPU::OPCode_50()
{
	LD_r1_r2(D, B);
}

void CPU::OPCode_51()
{
	LD_r1_r2(D, C);
}

void CPU::OPCode_52()
{
	LD_r1_r2(D, D);
}

void CPU::OPCode_53()
{
	LD_r1_r2(D, E);
}

void CPU::OPCode_54()
{
	LD_r1_r2(D, H);
}

void CPU::OPCode_55()
{
	LD_r1_r2(D, L);
}

void CPU::OPCode_56()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(D, addr);
}

void CPU::OPCode_58()
{
	LD_r1_r2(E, B);
}

void CPU::OPCode_59()
{
	LD_r1_r2(E, C);
}

void CPU::OPCode_5A()
{
	LD_r1_r2(E, D);
}

void CPU::OPCode_5B()
{
	LD_r1_r2(E, E);
}

void CPU::OPCode_5C()
{
	LD_r1_r2(E, H);
}

void CPU::OPCode_5D()
{
	LD_r1_r2(E, L);
}

void CPU::OPCode_5E()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(E, addr);
}

void CPU::OPCode_60()
{
	LD_r1_r2(H, B);
}

void CPU::OPCode_61()
{
	LD_r1_r2(H, C);
}

void CPU::OPCode_62()
{
	LD_r1_r2(H, D);
}

void CPU::OPCode_63()
{
	LD_r1_r2(H, E);
}

void CPU::OPCode_64()
{
	LD_r1_r2(H, H);
}

void CPU::OPCode_65()
{
	LD_r1_r2(H, L);
}

void CPU::OPCode_66()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(H, addr);
}

void CPU::OPCode_68()
{
	LD_r1_r2(L, B);
}

void CPU::OPCode_69()
{
	LD_r1_r2(L, C);
}

void CPU::OPCode_6A()
{
	LD_r1_r2(L, D);
}

void CPU::OPCode_6B()
{
	LD_r1_r2(L, E);
}

void CPU::OPCode_6C()
{
	LD_r1_r2(L, H);
}

void CPU::OPCode_6D()
{
	LD_r1_r2(L, L);
}

void CPU::OPCode_6E()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(L, addr);
}

void CPU::OPCode_70()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, B);
}

void CPU::OPCode_71()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, C);
}

void CPU::OPCode_72()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, D);
}

void CPU::OPCode_73()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, E);
}

void CPU::OPCode_74()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, H);
}

void CPU::OPCode_75()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, L);
}

void CPU::OPCode_36()
{
	byte n = readNextByte();
	word addr = HL.GetValue();
	LD_r1_r2(addr, (ByteRegister&)n);
}

void CPU::OPCode_0A()
{
	word addr = BC.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_1A()
{
	word addr = DE.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_FA()
{
	word addr = readNextWord();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_3E()
{
	LD_nn_n(A);
}

void CPU::OPCode_47()
{
	LD_r1_r2(B, A);
}

void CPU::OPCode_4F()
{
	LD_r1_r2(C, A);
}

void CPU::OPCode_57()
{
	LD_r1_r2(D, A);
}

void CPU::OPCode_5F()
{
	LD_r1_r2(E, A);
}

void CPU::OPCode_67()
{
	LD_r1_r2(H, A);
}

void CPU::OPCode_6F()
{
	LD_r1_r2(L, A);
}

void CPU::OPCode_02()
{
	word addr = BC.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_12()
{
	word addr = DE.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_77()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_EA()
{
	word addr = readNextWord();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_F2()
{
	word addr = 0xFF00 + C.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_E2()
{
	word addr = 0xFF00 + C.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_3A()
{
	OPCode_7E();
	HL.Decrement();
}

void CPU::OPCode_32()
{
	OPCode_77();
	HL.Decrement();
}

void CPU::OPCode_2A()
{
	OPCode_7E();
	HL.Increment();
}

void CPU::OPCode_22()
{
	OPCode_77();
	HL.Increment();
}

void CPU::OPCode_E0()
{
	byte val = readNextByte();
	word addr = 0xFF00 + val;
	LD_r1_r2(addr, A);
}

void CPU::OPCode_F0()
{
	byte val = readNextByte();
	word addr = 0xFF00 + val;
	LD_r1_r2(A, addr);
}

void CPU::OPCode_01()
{
	LD_n_nn(BC);
}

void CPU::OPCode_11()
{
	LD_nn_n(DE);
}

void CPU::OPCode_21()
{
	LD_nn_n(HL);
}

void CPU::OPCode_31()
{
	LD_nn_n(SP);
}

void CPU::OPCode_F9()
{
	LD_r1_r2(SP, HL);
}

void CPU::OPCode_F8()
{
	LD_HL_SP_n();
}

void CPU::OPCode_08()
{
	word addr = readNextWord();
	LD_r1_r2(addr, SP);
}

void CPU::OPCode_F5()
{
	PUSH(AF);
}

void CPU::OPCode_C5()
{
	PUSH(BC);
}

void CPU::OPCode_D5()
{
	PUSH(DE);
}

void CPU::OPCode_E5()
{
	PUSH(HL);
}

void CPU::OPCode_F1()
{
	POP(AF);
}

void CPU::OPCode_C1()
{
	POP(BC);
}

void CPU::OPCode_D1()
{
	POP(DE);
}

void CPU::OPCode_E1()
{
	POP(HL);
}

void CPU::OPCode_87()
{
	byte val = static_cast<byte>(A.GetValue());
	ADD(val);
}

void CPU::OPCode_80()
{
	byte val = static_cast<byte>(B.GetValue());
	ADD(val);
}

void CPU::OPCode_81()
{
	byte val = static_cast<byte>(C.GetValue());
	ADD(val);
}

void CPU::OPCode_82()
{
	byte val = static_cast<byte>(D.GetValue());
	ADD(val);
}

void CPU::OPCode_83()
{
	byte val = static_cast<byte>(E.GetValue());
	ADD(val);
}

void CPU::OPCode_84()
{
	byte val = static_cast<byte>(H.GetValue());
	ADD(val);
}

void CPU::OPCode_85()
{
	byte val = static_cast<byte>(L.GetValue());
	ADD(val);
}

void CPU::OPCode_86()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ADD(val);
}

void CPU::OPCode_C6()
{
	byte val = readNextByte();
	ADD(val);
}

void CPU::OPCode_8F()
{
	byte val = static_cast<byte>(A.GetValue());
	ADC(val);
}

void CPU::OPCode_88()
{
	byte val = static_cast<byte>(B.GetValue());
	ADC(val);
}

void CPU::OPCode_89()
{
	byte val = static_cast<byte>(C.GetValue());
	ADC(val);
}

void CPU::OPCode_8A()
{
	byte val = static_cast<byte>(D.GetValue());
	ADC(val);
}

void CPU::OPCode_8B()
{
	byte val = static_cast<byte>(E.GetValue());
	ADC(val);
}

void CPU::OPCode_8C()
{
	byte val = static_cast<byte>(H.GetValue());
	ADC(val);
}

void CPU::OPCode_8D()
{
	byte val = static_cast<byte>(L.GetValue());
	ADC(val);
}

void CPU::OPCode_8E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ADC(val);
}

void CPU::OPCode_CE()
{
	byte val = readNextByte();
	ADC(val);
}

void CPU::OPCode_97()
{
	byte val = static_cast<byte>(A.GetValue());
	SUB(val);
}

void CPU::OPCode_90()
{
	byte val = static_cast<byte>(B.GetValue());
	SUB(val);
}

void CPU::OPCode_91()
{
	byte val = static_cast<byte>(C.GetValue());
	SUB(val);
}

void CPU::OPCode_92()
{
	byte val = static_cast<byte>(D.GetValue());
	SUB(val);
}

void CPU::OPCode_93()
{
	byte val = static_cast<byte>(E.GetValue());
	SUB(val);
}

void CPU::OPCode_94()
{
	byte val = static_cast<byte>(H.GetValue());
	SUB(val);
}

void CPU::OPCode_95()
{
	byte val = static_cast<byte>(L.GetValue());
	SUB(val);
}

void CPU::OPCode_96()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SUB(val);
}

void CPU::OPCode_D6()
{
	byte val = readNextByte();
	SUB(val);
}

void CPU::OPCode_9F()
{
	byte val = static_cast<byte>(A.GetValue());
	SBC(val);
}

void CPU::OPCode_98()
{
	byte val = static_cast<byte>(B.GetValue());
	SBC(val);
}

void CPU::OPCode_99()
{
	byte val = static_cast<byte>(C.GetValue());
	SBC(val);
}

void CPU::OPCode_9A()
{
	byte val = static_cast<byte>(D.GetValue());
	SBC(val);
}

void CPU::OPCode_9B()
{
	byte val = static_cast<byte>(E.GetValue());
	SBC(val);
}

void CPU::OPCode_9C()
{
	byte val = static_cast<byte>(H.GetValue());
	SBC(val);
}

void CPU::OPCode_9D()
{
	byte val = static_cast<byte>(L.GetValue());
	SBC(val);
}

void CPU::OPCode_9E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SBC(val);
}

void CPU::OPCode_DE()
{
	byte val = readNextByte();
	SBC(val);
}

void CPU::OPCode_A7()
{
	byte val = static_cast<byte>(A.GetValue());
	AND(val);
}

void CPU::OPCode_A0()
{
	byte val = static_cast<byte>(B.GetValue());
	AND(val);
}

void CPU::OPCode_A1()
{
	byte val = static_cast<byte>(C.GetValue());
	AND(val);
}

void CPU::OPCode_A2()
{
	byte val = static_cast<byte>(D.GetValue());
	AND(val);
}

void CPU::OPCode_A3()
{
	byte val = static_cast<byte>(E.GetValue());
	AND(val);
}

void CPU::OPCode_A4()
{
	byte val = static_cast<byte>(H.GetValue());
	AND(val);
}

void CPU::OPCode_A5()
{
	byte val = static_cast<byte>(L.GetValue());
	AND(val);
}

void CPU::OPCode_A6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	AND(val);
}

void CPU::OPCode_E6()
{
	byte val = readNextByte();
	AND(val);
}

void CPU::OPCode_B7()
{
	byte val = static_cast<byte>(A.GetValue());
	OR(val);
}

void CPU::OPCode_B0()
{
	byte val = static_cast<byte>(B.GetValue());
	OR(val);
}

void CPU::OPCode_B1()
{
	byte val = static_cast<byte>(C.GetValue());
	OR(val);
}

void CPU::OPCode_B2()
{
	byte val = static_cast<byte>(D.GetValue());
	OR(val);
}

void CPU::OPCode_B3()
{
	byte val = static_cast<byte>(E.GetValue());
	OR(val);
}

void CPU::OPCode_B4()
{
	byte val = static_cast<byte>(H.GetValue());
	OR(val);
}

void CPU::OPCode_B5()
{
	byte val = static_cast<byte>(L.GetValue());
	OR(val);
}

void CPU::OPCode_B6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	OR(val);
}

void CPU::OPCode_F6()
{
	byte val = readNextByte();
	OR(val);
}

void CPU::OPCode_AF()
{
	byte val = static_cast<byte>(A.GetValue());
	XOR(val);
}

void CPU::OPCode_A8()
{
	byte val = static_cast<byte>(B.GetValue());
	XOR(val);
}

void CPU::OPCode_A9()
{
	byte val = static_cast<byte>(C.GetValue());
	XOR(val);
}

void CPU::OPCode_AA()
{
	byte val = static_cast<byte>(D.GetValue());
	XOR(val);
}

void CPU::OPCode_AB()
{
	byte val = static_cast<byte>(E.GetValue());
	XOR(val);
}

void CPU::OPCode_AC()
{
	byte val = static_cast<byte>(H.GetValue());
	XOR(val);
}

void CPU::OPCode_AD()
{
	byte val = static_cast<byte>(L.GetValue());
	XOR(val);
}

void CPU::OPCode_AE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	XOR(val);
}

void CPU::OPCode_EE()
{
	byte val = readNextByte();
	XOR(val);
}

void CPU::OPCode_BF()
{
	byte val = static_cast<byte>(A.GetValue());
	CP(val);
}

void CPU::OPCode_B8()
{
	byte val = static_cast<byte>(B.GetValue());
	CP(val);
}

void CPU::OPCode_B9()
{
	byte val = static_cast<byte>(C.GetValue());
	CP(val);
}

void CPU::OPCode_BA()
{
	byte val = static_cast<byte>(D.GetValue());
	CP(val);
}

void CPU::OPCode_BB()
{
	byte val = static_cast<byte>(E.GetValue());
	CP(val);
}

void CPU::OPCode_BC()
{
	byte val = static_cast<byte>(H.GetValue());
	CP(val);
}

void CPU::OPCode_BD()
{
	byte val = static_cast<byte>(L.GetValue());
	CP(val);
}

void CPU::OPCode_BE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	CP(val);
}

void CPU::OPCode_FE()
{
	byte val = readNextByte();
	CP(val);
}

void CPU::OPCode_3C()
{
	INC(A);
}

void CPU::OPCode_04()
{
	INC(B);
}

void CPU::OPCode_0C()
{
	INC(C);
}


void CPU::OPCode_14()
{
	INC(D);
}


void CPU::OPCode_1C()
{
	INC(E);
}


void CPU::OPCode_24()
{
	INC(H);
}


void CPU::OPCode_2C()
{
	INC(L);
}


void CPU::OPCode_34()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	INC((ByteRegister&)val); // the incremented value will be lost as its just a copy, however this call will adjust the cpu flag register
	val += 1;
	m_MMU.Write(addr, val);
}

void CPU::OPCode_3D()
{
	DEC(A);
}

void CPU::OPCode_05()
{
	DEC(B);
}

void CPU::OPCode_0D()
{
	DEC(C);
}

void CPU::OPCode_15()
{
	DEC(D);
}

void CPU::OPCode_1D()
{
	DEC(E);
}

void CPU::OPCode_25()
{
	DEC(H);
}

void CPU::OPCode_2D()
{
	DEC(L);
}

void CPU::OPCode_35()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	DEC((ByteRegister&)val); // the decremented value will be lost as its just a copy, however this call will adjust the cpu flag register
	val -= 1;
	m_MMU.Write(addr, val);
}

void CPU::OPCode_09()
{
	word val = BC.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_19()
{
	word val = DE.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_29()
{
	word val = HL.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_39()
{
	word val = SP.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_E8()
{
	ADD_SP();
}

void CPU::OPCode_03()
{
	INC_no_flags(BC);
}

void CPU::OPCode_13()
{
	INC_no_flags(DE);
}

void CPU::OPCode_23()
{
	INC_no_flags(HL);
}

void CPU::OPCode_33()
{
	INC_no_flags(SP);
}

void CPU::OPCode_0B()
{
	DEC_no_flags(BC);
}

void CPU::OPCode_1B()
{
	DEC_no_flags(DE);
}

void CPU::OPCode_2B()
{
	DEC_no_flags(HL);
}

void CPU::OPCode_3B()
{
	DEC_no_flags(SP);
}

void CPU::OPCode_27()
{
	DAA();
}
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
	byte val = i_SrcRegister.GetValue();
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
	byte highVal = i_RegisterPair.GetHighRegister().GetValue();
	m_MMU.Write(addr, highVal);

	SP.Decrement();
	addr = SP.GetValue();
	byte lowVal = i_RegisterPair.GetLowRegister().GetValue();
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
	byte aVal = A.GetValue();
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
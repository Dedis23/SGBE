#include "cpu.h"

CPU::CPU(MMU& i_MMU) : m_MMU(i_MMU), AF(A, F), BC(B, C), DE(D, E), HL(H, L), m_IME(false) {}

void CPU::Step()
{
	// read next instruction opcode
	byte OPCode = m_MMU.Read(PC.GetValue());
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

/* OPCodes */

/*
	Operation:
	LD nn, n

	Description:
	Put value n (memory) into nn.
*/
void CPU::LD_nn_n(ByteRegister& i_DestOperand)
{
	byte n = readNextByte();
	i_DestOperand.SetValue(n);
}


/*
	Operation:
	LD r1, r2

	Description:
	Put value r2 into r1.

*/
void CPU::LD_r1_r2(ByteRegister& i_DestOperand, const ByteRegister& i_SrcOperand)
{
	byte val = i_SrcOperand.GetValue();
	i_DestOperand.SetValue(val);
}

void CPU::LD_r1_r2(ByteRegister& i_DestOperand, const WordAddress& i_SrcOperand)
{
	byte val = m_MMU.Read(i_SrcOperand);
	i_DestOperand.SetValue(val);
}

void CPU::LD_r1_r2(const WordAddress& i_DestOperand, ByteRegister& i_SrcOperand)
{
	byte val = i_SrcOperand.GetValue();
	m_MMU.Write(i_DestOperand, val);
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
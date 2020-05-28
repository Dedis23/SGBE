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
	Put value n into nn

	nn = B, C, D, E, H, L
	n = 8 bit immediate value
*/
void CPU::LD_nn_n(ByteRegister& i_Operand)
{
	byte n = readNextByte();
	i_Operand.SetValue(n);
}

const std::vector<CPU::OPCodeData> CPU::m_OPCodeDataMap
{
	{ &OPCode_06, "LD B, n", 8 }, // 06
	{ &OPCode_0E, "LD C, n", 8 }, // 0E
	{ &OPCode_16, "LD D, n", 8 }, // 16
	{ &OPCode_1E, "LD E, n", 8 }, // 1E
	{ &OPCode_26, "LD H, n", 8 }, // 26
	{ &OPCode_2E, "LD L, n", 8 }, // 2E
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
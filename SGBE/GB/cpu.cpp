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
	byte highByte = readNextByte();
	byte lowByte = readNextByte();
	return static_cast<word>(highByte << 8 | lowByte);
}

/* OPCodes */

/*
	Operation:
	LD nn, n

	Description:
	Put value n into nn

	nn = B, C, D, E, H, L, BC, DE, HL, SP
	n = 8 bit immediate value
*/
void LD_nn_n(ByteRegister& i_Operand)
{

}

const std::vector<CPU::OPCodeData> CPU::m_OPCodeDataMap
{
	{ &OPCode_06, "LD B, n", 8 } // 06
};

void CPU::OPCode_06()
{
	LD_nn_n(B);
}
#include "cpu.h"

CPU::CPU(MMU& i_MMU) : m_MMU(i_MMU), AF(A, F), BC(B, C), DE(D, E), HL(H, L), m_IME(false) {}

void CPU::Step()
{
	// read next byte from memory (whether its an instruction or a value from the memory)
	byte opcode = m_MMU.Read(PC.GetValue());

	//cout << m_OPCodeMap[0].Name << endl;
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

uint32_t CPU::stam(byte operand)
{
	cout << "in stam" << endl;
	return 999;
}

const std::vector<CPU::OPCode> CPU::m_OPCodeMap
{
	{ &CPU::stam, "stam" }
};
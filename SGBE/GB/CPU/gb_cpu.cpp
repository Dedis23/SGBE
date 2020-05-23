#include "gb_cpu.h"

GBCPU::GBCPU() : AF(A, F), BC(B, C), DE(D, E), HL(H, L), m_IME(false) {}

void GBCPU::Step()
{
	//byte instruction = MMU.
}

void GBCPU::Reset()
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
#include "registers.h"

ByteRegister::ByteRegister(uint8_t i_Value) : m_Value(i_Value) {}

void ByteRegister::SetRegister(uint8_t i_NewValue)
{
	m_Value = i_NewValue;
}

void ByteRegister::SetBit(uint8_t i_BitNumber, bool i_IsRaise)
{
	if (i_IsRaise)	m_Value |= 1 << (i_BitNumber - 1);
	else m_Value &= ~(1 << (i_BitNumber - 1));
}

bool ByteRegister::GetBit(uint8_t i_BitNumber) const
{
	return (m_Value & (1 << (i_BitNumber - 1))) > 0 ? true : false;
}

void ByteRegister::Clear()
{
	m_Value = 0;
}

void ByteRegister::Increment()
{
	m_Value += 1;
}

void ByteRegister::Decrement()
{
	m_Value -= 1;
}
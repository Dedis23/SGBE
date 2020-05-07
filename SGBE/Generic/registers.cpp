#include "registers.h"

ByteRegister::ByteRegister(uint8_t i_Value) : m_Value(i_Value) {}

void ByteRegister::SetValue(uint8_t i_NewValue)
{
	m_Value = i_NewValue;
}

uint8_t ByteRegister::GetValue()
{
	return m_Value;
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

WordRegister::WordRegister(uint16_t i_Value) : m_Value(i_Value) {}

void WordRegister::SetValue(uint16_t i_NewValue)
{
	m_Value = i_NewValue;
}

uint16_t WordRegister::GetValue()
{
	return m_Value;
}

void WordRegister::SetLowByte(uint8_t i_NewLowByte)
{
	m_Value &= 0xFF00;
	m_Value |= i_NewLowByte;
}

uint8_t WordRegister::GetLowByte() const
{
	return static_cast<uint8_t>(m_Value);
}

void WordRegister::SetHighByte(uint8_t i_NewHighByte)
{
	m_Value &= 0x00FF;
	m_Value |= static_cast<uint16_t>(i_NewHighByte << 8);
}

uint8_t WordRegister::GetHighByte() const
{
	return static_cast<uint8_t>(m_Value >> 8);
}

void WordRegister::SetBit(uint8_t i_BitNumber, bool i_IsRaise)
{
	if (i_IsRaise)	m_Value |= 1 << (i_BitNumber - 1);
	else m_Value &= ~(1 << (i_BitNumber - 1));
}

bool WordRegister::GetBit(uint8_t i_BitNumber) const
{
	return (m_Value & (1 << (i_BitNumber - 1))) > 0 ? true : false;
}

void WordRegister::Clear()
{
	m_Value = 0;
}

void WordRegister::Increment()
{
	m_Value += 1;
}

void WordRegister::Decrement()
{
	m_Value -= 1;
}
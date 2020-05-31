#include "registers.h"

IRegister::IRegister(word i_Value) : m_Value(i_Value) {}

void IRegister::SetBit(byte i_BitNumber, bool i_IsRaise)
{
	if (i_IsRaise)	m_Value |= 1 << (i_BitNumber);
	else m_Value &= ~(1 << (i_BitNumber));
}

bool IRegister::GetBit(byte i_BitNumber) const
{
	return (m_Value & (1 << i_BitNumber)) > 0 ? true : false;
}

void IRegister::Clear()
{
	m_Value = 0;
}

ByteRegister::ByteRegister(byte i_Value) : IRegister(i_Value) {}

void ByteRegister::SetValue(word i_NewValue)
{
	m_Value = static_cast<byte>(i_NewValue);
}

word ByteRegister::GetValue() const
{
	return static_cast<byte>(m_Value);
}

void ByteRegister::Increment()
{
	m_Value = static_cast<byte>(m_Value + 1);
}

void ByteRegister::Decrement()
{
	m_Value = static_cast<byte>(m_Value - 1);
}

WordRegister::WordRegister(word i_Value) : IRegister(i_Value) {}

void WordRegister::SetValue(word i_NewValue)
{
	m_Value = i_NewValue;
}

word WordRegister::GetValue() const
{
	return m_Value;
}

void WordRegister::SetLowByte(byte i_NewLowByte)
{
	m_Value &= 0xFF00;
	m_Value |= i_NewLowByte;
}

byte WordRegister::GetLowByte() const
{
	return static_cast<byte>(m_Value);
}

void WordRegister::SetHighByte(byte i_NewHighByte)
{
	m_Value &= 0x00FF;
	m_Value |= static_cast<word>(i_NewHighByte << 8);
}

byte WordRegister::GetHighByte() const
{
	return static_cast<byte>(m_Value >> 8);
}

void WordRegister::Increment()
{
	m_Value += 1;
}

void WordRegister::Decrement()
{
	m_Value -= 1;
}

Pair8BRegisters::Pair8BRegisters(ByteRegister& i_HighByteRegister, ByteRegister& i_LowByteRegister)
	: m_HighByteRegister(i_HighByteRegister), m_LowByteRegister(i_LowByteRegister) {}

void Pair8BRegisters::SetValue(word i_NewValue)
{
	m_LowByteRegister.SetValue(static_cast<byte>(i_NewValue));
	m_HighByteRegister.SetValue(static_cast<byte>(i_NewValue >> 8));
}

word Pair8BRegisters::GetValue() const
{
	return (static_cast<word>(m_HighByteRegister.GetValue() << 8)) | m_LowByteRegister.GetValue();
}

void Pair8BRegisters::SetBit(byte i_BitNumber, bool i_IsRaise)
{
	if (i_BitNumber <= 15 && i_BitNumber > 7)
	{
		m_HighByteRegister.SetBit(i_BitNumber - 8, i_IsRaise);
	}
	else if (i_BitNumber <= 7)
	{
		m_LowByteRegister.SetBit(i_BitNumber, i_IsRaise);
	}
}

bool Pair8BRegisters::GetBit(byte i_BitNumber) const
{
	bool res = false;
	if (i_BitNumber <= 15 && i_BitNumber > 7)
	{
		res = m_HighByteRegister.GetBit(i_BitNumber - 8);
	}
	else if (i_BitNumber <= 7)
	{
		res = m_LowByteRegister.GetBit(i_BitNumber);
	}
	return res;
}

void Pair8BRegisters::Clear()
{
	m_LowByteRegister.Clear();
	m_HighByteRegister.Clear();
}

void Pair8BRegisters::Increment()
{
	SetValue(GetValue() + 1);
}

void Pair8BRegisters::Decrement()
{
	SetValue(GetValue() - 1);
}

const ByteRegister& Pair8BRegisters::GetHighRegister() const
{
	return m_HighByteRegister;
}

const ByteRegister& Pair8BRegisters::GetLowRegister() const
{
	return m_LowByteRegister;
}

FlagRegister::FlagRegister(byte i_Value) : ByteRegister(i_Value) {}

void FlagRegister::SetZ(bool i_IsRaise)
{
	SetBit(7, i_IsRaise);
}

bool FlagRegister::GetZ() const
{
	return GetBit(7);
}

void FlagRegister::SetN(bool i_IsRaise)
{
	SetBit(6, i_IsRaise);
}

bool FlagRegister::GetN() const
{
	return GetBit(6);
}

void FlagRegister::SetH(bool i_IsRaise)
{
	SetBit(5, i_IsRaise);
}

bool FlagRegister::GetH() const
{
	return GetBit(5);
}

void FlagRegister::SetC(bool i_IsRaise)
{
	SetBit(4, i_IsRaise);
}

bool FlagRegister::GetC() const
{
	return GetBit(4);
}
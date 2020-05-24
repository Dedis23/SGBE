#include "registers.h"

ByteRegister::ByteRegister(byte i_Value) : m_Value(i_Value) {}

void ByteRegister::SetValue(byte i_NewValue)
{
	m_Value = i_NewValue;
}

byte ByteRegister::GetValue() const
{
	return m_Value;
}

void ByteRegister::SetBit(byte i_BitNumber, bool i_IsRaise)
{
	if (i_IsRaise)	m_Value |= 1 << (i_BitNumber - 1);
	else m_Value &= ~(1 << (i_BitNumber - 1));
}

bool ByteRegister::GetBit(byte i_BitNumber) const
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

WordRegister::WordRegister(word i_Value) : m_Value(i_Value) {}

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

void WordRegister::SetBit(byte i_BitNumber, bool i_IsRaise)
{
	if (i_IsRaise)	m_Value |= 1 << (i_BitNumber - 1);
	else m_Value &= ~(1 << (i_BitNumber - 1));
}

bool WordRegister::GetBit(byte i_BitNumber) const
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

ByteRegister& Pair8BRegisters::GetHighRegister()
{
	return m_HighByteRegister;
}

ByteRegister& Pair8BRegisters::GetLowRegister()
{
	return m_LowByteRegister;
}

FlagRegister::FlagRegister(byte i_Value) : ByteRegister(i_Value) {}

void FlagRegister::SetZFlag(bool i_IsRaise)
{
	SetBit(8, i_IsRaise);
}

bool FlagRegister::GetZFlag() const
{
	return GetBit(8);
}

void FlagRegister::SetNFlag(bool i_IsRaise)
{
	SetBit(7, i_IsRaise);
}

bool FlagRegister::GetNFlag() const
{
	return GetBit(7);
}

void FlagRegister::SetHFlag(bool i_IsRaise)
{
	SetBit(6, i_IsRaise);
}

bool FlagRegister::GetHFlag() const
{
	return GetBit(6);
}

void FlagRegister::SetCFlag(bool i_IsRaise)
{
	SetBit(5, i_IsRaise);
}

bool FlagRegister::GetCFlag() const
{
	return GetBit(5);
}
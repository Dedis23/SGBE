#include "registers.h"

ByteRegister::ByteRegister(byte i_Value) : m_Value(i_Value) {}

inline void ByteRegister::SetValue(byte i_Value)
{
	m_Value = i_Value;
}

inline byte ByteRegister::GetValue() const
{
	return m_Value;
}

inline void ByteRegister::Increment()
{
	m_Value += 1;
}

inline void ByteRegister::Decrement()
{
	m_Value -= 1;
}

inline void ByteRegister::SetBit(byte i_BitNumber, bool i_IsRaise)
{
	if (i_IsRaise)	m_Value |= 1 << (i_BitNumber);
	else m_Value &= ~(1 << (i_BitNumber));
}

inline bool ByteRegister::GetBit(byte i_BitNumber) const
{
	return (m_Value & (1 << i_BitNumber)) > 0 ? true : false;
}

inline void ByteRegister::Clear()
{
	m_Value = 0;
}

WordRegister::WordRegister(word i_Value) : m_Value(i_Value) {}

inline void WordRegister::SetValue(word i_Value)
{
	m_Value = i_Value;
}

inline word WordRegister::GetValue() const
{
	return m_Value;
}

inline void WordRegister::SetLowByte(byte i_LowByte)
{
	m_Value &= 0xFF00;
	m_Value |= i_LowByte;
}

inline byte WordRegister::GetLowByte() const
{
	return static_cast<byte>(m_Value);
}

inline void WordRegister::SetHighByte(byte i_HighByte)
{
	m_Value &= 0x00FF;
	m_Value |= static_cast<word>(i_HighByte << 8);
}

inline byte WordRegister::GetHighByte() const
{
	return static_cast<byte>(m_Value >> 8);
}

inline void WordRegister::Increment()
{
	m_Value += 1;
}

inline void WordRegister::Decrement()
{
	m_Value -= 1;
}

inline void WordRegister::SetBit(byte i_BitNumber, bool i_IsRaise)
{
	if (i_IsRaise)	m_Value |= 1 << (i_BitNumber);
	else m_Value &= ~(1 << (i_BitNumber));
}

inline bool WordRegister::GetBit(byte i_BitNumber) const
{
	return (m_Value & (1 << i_BitNumber)) > 0 ? true : false;
}

inline void WordRegister::Clear()
{
	m_Value = 0;
}

Pair8BRegisters::Pair8BRegisters(ByteRegister& i_HighByteRegister, ByteRegister& i_LowByteRegister)
	: m_HighByteRegister(i_HighByteRegister), m_LowByteRegister(i_LowByteRegister) {}

inline void Pair8BRegisters::SetValue(word i_NewValue)
{
	m_LowByteRegister.SetValue(static_cast<byte>(i_NewValue));
	m_HighByteRegister.SetValue(static_cast<byte>(i_NewValue >> 8));
}

inline word Pair8BRegisters::GetValue() const
{
	return (static_cast<word>(m_HighByteRegister.GetValue() << 8)) | m_LowByteRegister.GetValue();
}

inline void Pair8BRegisters::SetBit(byte i_BitNumber, bool i_IsRaise)
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

inline bool Pair8BRegisters::GetBit(byte i_BitNumber) const
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

void Pair8BRegisters::Increment()
{
	SetValue(GetValue() + 1);
}

void Pair8BRegisters::Decrement()
{
	SetValue(GetValue() - 1);
}

inline void Pair8BRegisters::Clear()
{
	m_LowByteRegister.Clear();
	m_HighByteRegister.Clear();
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
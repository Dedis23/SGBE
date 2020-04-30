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

FlagRegister::FlagRegister(uint8_t i_Value) : ByteRegister(i_Value) {}

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

//WordRegister::WordRegister(uint16_t i_Value) : m_Value(i_Value) {}
//
//void WordRegister::SetBit(uint8_t i_BitNumber, bool i_IsRaise)
//{
//	if (i_IsRaise)	m_Value |= 1 << (i_BitNumber - 1);
//	else m_Value &= ~(1 << (i_BitNumber - 1));
//}
//
//bool WordRegister::GetBit(uint8_t i_BitNumber) const
//{
//	return (m_Value & (1 << (i_BitNumber - 1))) > 0 ? true : false;
//}
//
//void WordRegister::Clear()
//{
//	m_Value = 0;
//}
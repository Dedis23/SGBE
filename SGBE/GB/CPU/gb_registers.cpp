#include "gb_registers.h"

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

PairRegisters::PairRegisters(ByteRegister& i_HighByteRegister, ByteRegister& i_LowByteRegister) 
	: m_HighByteRegister(i_HighByteRegister), m_LowByteRegister(i_LowByteRegister) {}

uint16_t PairRegisters::GetValue()
{
	return (static_cast<uint16_t>(m_HighByteRegister.GetValue() << 8)) | m_LowByteRegister.GetValue();
}

void PairRegisters::SetValue(uint16_t i_NewValue)
{
	m_LowByteRegister.SetValue(static_cast<uint8_t>(i_NewValue));
	m_HighByteRegister.SetValue(static_cast<uint8_t>(i_NewValue >> 8));
}

ByteRegister& PairRegisters::GetHighRegister()
{
	return m_HighByteRegister;
}

ByteRegister& PairRegisters::GetLowRegister()
{
	return m_LowByteRegister;
}
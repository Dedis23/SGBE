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
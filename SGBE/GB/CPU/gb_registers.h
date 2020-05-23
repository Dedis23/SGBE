/**********************************************************************
 *			Copyright (C) 2020 Dedi Sidi			                  *
 *														              *
 *			register classes that are specific for the gameboy        *
 **********************************************************************/

#ifndef __GB_REGISTERS_H
#define __GB_REGISTERS_H

#include "../../Generic/registers.h"

/*
	The GB CPU has one flag register and it uses its 4 high bits:
	7 6 5 4 3 2 1 0
	Z N H C 0 0 0 0
	Zero Flag (Z): This bit is set when the result of a math operation is zero or two values match when using the CP instruction.
	Subtract Flag (N): This bit is set if a subtraction was performed in the last math instruction.
	Half Carry Flag (H): This bit is set if a carry occurred from the lower nibble in the last math operation.
	Carry Flag (C): This bit is set if a carry occurred from the last math operation or if register A is the smaller value when executing the CP instruction.
*/
class FlagRegister : public ByteRegister
{
public:
	FlagRegister(byte i_Value = 0);
	virtual ~FlagRegister() = default;
	FlagRegister(const FlagRegister&) = delete;
	FlagRegister& operator=(const FlagRegister&) = delete;

	void SetZFlag(bool i_IsRaise);
	bool GetZFlag() const;
	void SetNFlag(bool i_IsRaise);
	bool GetNFlag() const;
	void SetHFlag(bool i_IsRaise);
	bool GetHFlag() const;
	void SetCFlag(bool i_IsRaise);
	bool GetCFlag() const;
};

#endif
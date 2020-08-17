/****************************************************************
 *		Created by: Dedi Sidi, 2020								*
 *																*
 *		Generic registers classes of various sizes				*
 *		as well as registers that are specific for the gameboy 	*
 ****************************************************************/

#pragma once
#include <iostream>
#include "utility.h"

/*
	A general 8-bit register
*/
class ByteRegister
{
public:
	explicit ByteRegister(byte i_Value = 0);
	virtual ~ByteRegister() = default;

	virtual void SetValue(byte i_Value);
	virtual byte GetValue() const;
	virtual void Increment();
	virtual void Decrement();
	virtual void SetBit(byte i_BitNumber, bool i_IsRaise);
	virtual bool GetBit(byte i_BitNumber) const;
	virtual void Clear();
protected:
	byte m_Value;
};

/*
	A general 16-bit register
*/
class WordRegister
{
public:
	explicit WordRegister(word i_Value = 0);
	virtual ~WordRegister() = default;

	virtual void SetValue(word i_Value);
	virtual word GetValue() const;
	virtual void SetLowByte(byte i_LowByte);
	virtual byte GetLowByte() const;
	virtual void SetHighByte(byte i_HighByte);
	virtual byte GetHighByte() const;
	virtual void Increment();
	virtual void Decrement();
	virtual void SetBit(byte i_BitNumber, bool i_IsRaise);
	virtual bool GetBit(byte i_BitNumber) const;
	virtual void Clear();

protected:
	word m_Value;
};

/*
	16-bit register that is consists of 2x8-bit registers paired together
*/
class Pair8BRegisters
{
public:
	explicit Pair8BRegisters(ByteRegister& i_HighByteRegister, ByteRegister& i_LowByteRegister);
	virtual ~Pair8BRegisters() = default;
	Pair8BRegisters(const Pair8BRegisters&) = delete;
	Pair8BRegisters& operator=(const Pair8BRegisters&) = delete;

	void SetValue(word i_NewValue);
	word GetValue() const;
	void SetBit(byte i_BitNumber, bool i_IsRaise);
	bool GetBit(byte i_BitNumber) const;
	void Increment();
	void Decrement();
	void Clear();
	ByteRegister& GetHighRegister();
	ByteRegister& GetLowRegister();
private:
	ByteRegister& m_HighByteRegister;
	ByteRegister& m_LowByteRegister;
};

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
	explicit FlagRegister(byte i_Value = 0);
	virtual ~FlagRegister() = default;
	FlagRegister(const FlagRegister&) = delete;
	FlagRegister& operator=(const FlagRegister&) = delete;

	void SetZ(bool i_IsRaise);
	bool GetZ() const;
	void SetN(bool i_IsRaise);
	bool GetN() const;
	void SetH(bool i_IsRaise);
	bool GetH() const;
	void SetC(bool i_IsRaise);
	bool GetC() const;
};
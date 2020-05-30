/****************************************************************
 *		Copyright (C) 2020 Dedi Sidi							*
 *																*
 *		Generic registers classes of various sizes				*
 *		as well as registers that are specific for the gameboy 	*
 ****************************************************************/

#include <iostream>

#ifndef __REGISTERS_H
#define __REGISTERS_H

typedef uint8_t byte;
typedef int8_t sbyte;
typedef uint16_t word;

class IRegister
{
public:
	IRegister(word i_Value);
	virtual void SetValue(word i_NewValue) = 0;
	virtual word GetValue() const = 0;
	virtual void SetBit(byte i_BitNumber, bool i_IsRaise);
	virtual bool GetBit(byte i_BitNumber) const;
	virtual void Clear();
	virtual void Increment() = 0;
	virtual void Decrement() = 0;
protected:
	word m_Value = 0;
};

/*
	A general 8-bit register
*/
class ByteRegister : public IRegister
{
public:
	explicit ByteRegister(byte i_Value = 0);
	virtual ~ByteRegister() = default;

	void SetValue(word i_NewValue) override;
	word GetValue() const override;
	void Increment() override;
	void Decrement() override;
};

/*
	A general 16-bit register
*/
class WordRegister : public IRegister
{
public:
	WordRegister(word i_Value = 0);
	virtual ~WordRegister() = default;

	void SetValue(word i_NewValue);
	word GetValue() const;
	void SetLowByte(byte i_NewLowByte);
	byte GetLowByte() const;
	void SetHighByte(byte i_NewHighByte);
	byte GetHighByte() const;
	void Increment();
	void Decrement();
};

/*
	16-bit register that is consists of 2x8-bit registers paired together
*/
class Pair8BRegisters
{
public:
	Pair8BRegisters(ByteRegister& i_HighByteRegister, ByteRegister& i_LowByteRegister);
	virtual ~Pair8BRegisters() = default;
	Pair8BRegisters(const Pair8BRegisters&) = delete;
	Pair8BRegisters& operator=(const Pair8BRegisters&) = delete;

	void SetValue(word i_NewValue);
	word GetValue() const;
	void Clear();
	void Increment();
	void Decrement();
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
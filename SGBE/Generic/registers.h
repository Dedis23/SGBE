/************************************************************
 *			Copyright (C) 2020 Dedi Sidi					*
 *															*
 *			Generic registers classes of various sizes		*
 ************************************************************/

#ifndef __REGISTERS_H
#define __REGISTERS_H

#include <iostream>

typedef uint8_t byte;
typedef uint16_t word;

/*
	A general 8-bit register
*/
class ByteRegister
{
public:
	ByteRegister(byte i_Value = 0);
	virtual ~ByteRegister() = default;
	ByteRegister(const ByteRegister&) = delete;
	ByteRegister& operator=(const ByteRegister&) = delete;

	void SetValue(byte i_NewValue);
	byte GetValue() const;
	void SetBit(byte i_BitNumber, bool i_IsRaise);
	bool GetBit(byte i_BitNumber) const;
	void Clear();
	void Increment();
	void Decrement();
protected:
	byte m_Value = 0;
};

/*
	A general 16-bit register
*/
class WordRegister
{
public:
	WordRegister(word i_Value = 0);
	virtual ~WordRegister() = default;
	WordRegister(const WordRegister&) = delete;
	WordRegister& operator=(const WordRegister&) = delete;

	void SetValue(word i_NewValue);
	word GetValue() const;
	void SetLowByte(byte i_NewLowByte);
	byte GetLowByte() const;
	void SetHighByte(byte i_NewHighByte);
	byte GetHighByte() const;
	void SetBit(byte i_BitNumber, bool i_IsRaise);
	bool GetBit(byte i_BitNumber) const;
	void Clear();
	void Increment();
	void Decrement();
private:
	word m_Value = 0;
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
	ByteRegister& GetHighRegister();
	ByteRegister& GetLowRegister();
private:
	ByteRegister& m_HighByteRegister;
	ByteRegister& m_LowByteRegister;
};

#endif
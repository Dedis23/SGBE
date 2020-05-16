/************************************************************
 *			Copyright (C) 2020 Dedi Sidi					*
 *															*
 *			Generic registers classes of various sizes		*
 ************************************************************/

#ifndef __REGISTERS_H
#define __REGISTERS_H

#include <iostream>

/*
	A general 8-bit register
*/
class ByteRegister
{
public:
	ByteRegister(uint8_t i_Value = 0);
	virtual ~ByteRegister() = default;
	ByteRegister(const ByteRegister&) = delete;
	ByteRegister& operator=(const ByteRegister&) = delete;

	void SetValue(uint8_t i_NewValue);
	uint8_t GetValue() const;
	void SetBit(uint8_t i_BitNumber, bool i_IsRaise);
	bool GetBit(uint8_t i_BitNumber) const;
	void Clear();
	void Increment();
	void Decrement();
protected:
	uint8_t m_Value = 0;
};

/*
	A general 16-bit register
*/
class WordRegister
{
public:
	WordRegister(uint16_t i_Value = 0);
	virtual ~WordRegister() = default;
	WordRegister(const WordRegister&) = delete;
	WordRegister& operator=(const WordRegister&) = delete;

	void SetValue(uint16_t i_NewValue);
	uint16_t GetValue() const;
	void SetLowByte(uint8_t i_NewLowByte);
	uint8_t GetLowByte() const;
	void SetHighByte(uint8_t i_NewHighByte);
	uint8_t GetHighByte() const;
	void SetBit(uint8_t i_BitNumber, bool i_IsRaise);
	bool GetBit(uint8_t i_BitNumber) const;
	void Clear();
	void Increment();
	void Decrement();
private:
	uint16_t m_Value = 0;
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

	void SetValue(uint16_t i_NewValue);
	uint16_t GetValue() const;
	ByteRegister& GetHighRegister();
	ByteRegister& GetLowRegister();
private:
	ByteRegister& m_HighByteRegister;
	ByteRegister& m_LowByteRegister;
};

#endif
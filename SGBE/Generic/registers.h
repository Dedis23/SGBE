#ifndef __REGISTERS_H
#define __REGISTERS_H

#include <iostream>

/*
	A genreal 8-bit register class
*/
class ByteRegister
{
public:
	ByteRegister(uint8_t i_Value = 0);
	virtual ~ByteRegister() = default;
	ByteRegister(const ByteRegister&) = delete;
	ByteRegister& operator=(const ByteRegister&) = delete;

	void SetRegister(uint8_t i_NewValue);
	void SetBit(uint8_t i_BitNumber, bool i_IsRaise);
	bool GetBit(uint8_t i_BitNumber) const;
	void Clear();
	void Increment();
	void Decrement();
protected:
	uint8_t m_Value = 0;
};

#endif
/************************************************************
 *			Copyright (C) 2020 Dedi Sidi					*
 *															*
 *			Generic address classes of various sizes		*
 ************************************************************/

#ifndef __ADDRESS_H
#define __ADDRESS_H

#include "registers.h"

/*
	A general 16-bit address
*/
class WordAddress
{
public:
	WordAddress(uint16_t i_AddressLocation);
	WordAddress(const WordRegister& i_WordRegister);
	WordAddress(const Pair8BRegisters& i_Pair8BRegisters);
	virtual ~WordAddress() = default;

	uint16_t GetValue() const;
private:
	uint16_t m_Value = 0;
};

#endif
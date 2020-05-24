/************************************************************
 *			Copyright (C) 2020 Dedi Sidi					*
 *															*
 *			Generic address classes of various sizes		*
 ************************************************************/

#ifndef __ADDRESS_H
#define __ADDRESS_H

#include "../GB/registers.h"

/*
	A general 16-bit address
*/
class WordAddress
{
public:
	WordAddress(word i_AddressLocation);
	WordAddress(const WordRegister& i_WordRegister);
	WordAddress(const Pair8BRegisters& i_Pair8BRegisters);
	virtual ~WordAddress() = default;

	word GetValue() const;
	bool InRange(const WordAddress& i_Lower, const WordAddress& i_Upper) const;

private:
	word m_Value = 0;
};

#endif
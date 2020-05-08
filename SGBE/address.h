#ifndef __ADDRESS_H
#define __ADDRESS_H

#include "registers.h"

/*
	A genreal 16-bit address
*/
class WordAddress
{
public:
	WordAddress(uint16_t i_AddressLocation);
	WordAddress(const WordRegister& i_WordRegister);

private:
	uint16_t m_Value = 0;
};

#endif
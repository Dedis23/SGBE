/********************************************************
 *			Created by: Dedi Sidi, 2020				    *
 *														* 
 *			Utility functions and definitions           *
 ********************************************************/

#include <iostream>
#include "../Generic/logger.h"
#include "registers.h"

#ifndef __UTILITY_H
#define __UTILITY_H

const uint32_t TARGET_FRAME_RATE = 60;
const uint32_t GAMEBOY_SCREEN_WIDTH = 160;
const uint32_t GAMEBOY_SCREEN_HEIGHT = 144;

#define NOP

namespace bitwise {
	template<typename type>
	inline bool GetBit(size_t i_BitNumber, type i_Value)
	{
		return (i_Value & (1 << i_BitNumber)) > 0 ? true : false;
	}

	template<typename type>
	inline void SetBit(byte i_BitNumber, bool i_IsRaise, type& o_Value)
	{
		if (i_IsRaise)	o_Value |= 1 << (i_BitNumber);
		else o_Value &= ~(1 << (i_BitNumber));
	}
}

class WordAddress
{
public:
	WordAddress(word i_AddressLocation) : m_Value(i_AddressLocation) {}
	WordAddress(const WordRegister& i_WordRegister) : m_Value(i_WordRegister.GetValue()) {}
	WordAddress(const Pair8BRegisters& i_Pair8BRegisters) : m_Value(i_Pair8BRegisters.GetValue()) {}
	virtual ~WordAddress() = default;

	word GetValue() const
	{
		return m_Value;
	}

	bool checkRangeBounds(const WordAddress& i_LowerBound, const WordAddress& i_UpperBound) const
	{
		return m_Value >= i_LowerBound.GetValue() && m_Value <= i_UpperBound.GetValue();
	}

private:
	word m_Value = 0;
};

#endif
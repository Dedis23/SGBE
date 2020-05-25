/********************************************************
 *			Copyright (C) 2020 Dedi Sidi			    *
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
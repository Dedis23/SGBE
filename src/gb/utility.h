/************************************************
*		Created by: Dedi Sidi, 2020				*
*												*
*		Utility functions and definitions		*
*************************************************/

#pragma once
#include <iostream>
#include "../common/logger.h"

typedef uint8_t byte;
typedef int8_t sbyte;
typedef uint16_t word;

const float TARGET_FPS = 60;
const uint32_t GAMEBOY_SCREEN_WIDTH = 160;
const uint32_t GAMEBOY_SCREEN_HEIGHT = 144;

#define NOP

namespace bitwise 
{
	template<typename type>
	inline bool IsBitSet(size_t i_BitNumber, type i_Value)
	{
		return (i_Value & (1 << i_BitNumber)) > 0 ? true : false;
	}

	template<typename type>
	inline void SetBit(byte i_BitNumber, type& o_Value)
	{
		o_Value |= 1 << (i_BitNumber);
	}

	template<typename type>
	inline void ClearBit(byte i_BitNumber, type& o_Value)
	{
		o_Value &= ~(1 << (i_BitNumber));
	}
}
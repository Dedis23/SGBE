/*******************************************************
 *			Created by: Dedi Sidi, 2020                *
 *												       *
 *			PPU - Pixel Processing Unit                *
 *          The main graphics chip of the gameboy	   *
 *******************************************************/

#pragma once
#include "utility.h"

struct Pixel
{
    byte R;
    byte G;
    byte B;
};

const Pixel GAMEBOY_POCKET_PALLETE[4] = { { 255, 255, 255 }, { 192, 192, 192 }, { 96, 96, 96 }, { 0, 0, 0 } };

class PPU
{
public:
    PPU() = default;
	virtual ~PPU() = default;
    PPU(const PPU&) = delete;
    PPU& operator=(const PPU&) = delete;

    void Step(uint32_t& o_Cycles);
};
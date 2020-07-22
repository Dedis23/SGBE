/**********************************************************
 *			Created by: Dedi Sidi, 2020                   *
 *												          *
 *			GPU - The main graphics chip of the gameboy   *
 *********************************************************/

#pragma once
#include "utility.h"

struct Pixel
{
    byte R;
    byte G;
    byte B;
};

const Pixel GAMEBOY_POCKET_PALLETE[4] = { { 255, 255, 255 }, { 192, 192, 192 }, { 96, 96, 96 }, { 0, 0, 0 } };

class Gameboy;

class GPU
{
public:
    GPU(Gameboy& i_Gameboy);
	virtual ~GPU() = default;
    GPU(const GPU&) = delete;
    GPU& operator=(const GPU&) = delete;

    void Step(uint32_t& o_Cycles);

private:
    /* gameboy ref */
    Gameboy& m_Gameboy;
};
/************************************************
 *			Copyright (C) 2020 Dedi Sidi		*
 *												*
 *			The GPU of the gameboy				*
 ************************************************/

#include <string>
#include <vector>
#include "cartridge.h"
#include "utility.h"

#ifndef __GPU_H
#define __GPU_H

struct Pixel
{
    byte R;
    byte G;
    byte B;
};

const uint32_t TARGET_FRAME_RATE = 60;
const uint32_t GAMEBOY_SCREEN_WIDTH = 160;
const uint32_t GAMEBOY_SCREEN_HEIGHT = 144;

const Pixel GAMEBOY_POCKET_PALLETE[4] = { { 255, 255, 255 }, { 192, 192, 192 }, { 96, 96, 96 }, { 0, 0, 0 } };

class GPU
{
public:
    GPU() = default;
	virtual ~GPU() = default;
    GPU(const GPU&) = delete;
    GPU& operator=(const GPU&) = delete;

    void Step(uint32_t& o_Cycles);
};

#endif
/**********************************************************
 *			Created by: Dedi Sidi, 2020                   *
 *												          *
 *			GPU - The main graphics chip of the gameboy   *
 *********************************************************/

#pragma once
#include "utility.h"

/*  Most of the information were take from:                  *
 *  The Ultimate Game Boy Talk (33c3)                        *
 *  https://www.youtube.com/watch?v=HyzD8pNlpwI&t=           *
 *  codesligner:                                             *
 *  http://www.codeslinger.co.uk/pages/projects/gameboy.html *
 *  gameboy cpu manual pdf:                                  *
 *  http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf           */

 /* gpu (ppu) releated registers addresses in memory */
const word GPU_LCD_CONTROL_ADDR = 0xFF40; // R/W
const word GPU_LCDC_STATUS_ADDR = 0xFF41; // R/W
const word GPU_SCROLL_Y_ADDR = 0xFF42; // R/W
const word GPU_SCROLL_X_ADDR = 0xFF43; // R/W
const word GPU_LCDC_Y_CORRDINATE_ADDR = 0xFF44; // Read Only
const word GPU_LY_COMPARE_ADDR = 0xFF45; // R/W
const word GPU_DMA_TRANSFER_AND_START_ADDR = 0xFF46; // Write Only
const word GPU_BG_PALETTE_DATA_ADDR = 0xFF47; // R/W
const word GPU_OBJECT_PALETTE_0_DATA_ADDR = 0xFF48; // R/W
const word GPU_OBJECT_PALETTE_1_DATA_ADDR = 0xFF49; // R/W
const word GPU_WINDOW_Y_POSITION_ADDR = 0xFF4A; // R/W
const word GPU_WINDOW_X_POSITION_MINUS_7_ADDR = 0xFF4B; // R/W

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
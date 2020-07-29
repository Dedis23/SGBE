/**********************************************************
 *			Created by: Dedi Sidi, 2020                   *
 *												          *
 *			GPU - The main graphics chip of the gameboy   *
 *********************************************************/

#pragma once
#include "gameboy.h"
#include "utility.h"

/*  Most of the information were take from:                  *
 *  Pan Docs:                                                *
 *  https://gbdev.io/pandocs/#video-display                  *
 *  The Ultimate Game Boy Talk (33c3)                        *
 *  https://www.youtube.com/watch?v=HyzD8pNlpwI&t=           *
 *  codesligner:                                             *
 *  http://www.codeslinger.co.uk/pages/projects/gameboy.html *
 *  gameboy cpu manual pdf:                                  *
 *  http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf           */

 /* gpu (ppu) releated registers addresses in memory */
const word GPU_LCD_CONTROL_ADDR = 0xFF40;
const word GPU_LCDC_STATUS_ADDR = 0xFF41;
const word GPU_SCROLL_Y_ADDR = 0xFF42;
const word GPU_SCROLL_X_ADDR = 0xFF43;
const word GPU_LCDC_Y_CORRDINATE_ADDR = 0xFF44;
const word GPU_LY_COMPARE_ADDR = 0xFF45;
const word GPU_DMA_TRANSFER_AND_START_ADDR = 0xFF46;
const word GPU_BG_PALETTE_DATA_ADDR = 0xFF47;
const word GPU_OBJECT_PALETTE_0_DATA_ADDR = 0xFF48; 
const word GPU_OBJECT_PALETTE_1_DATA_ADDR = 0xFF49; 
const word GPU_WINDOW_Y_POSITION_ADDR = 0xFF4A;
const word GPU_WINDOW_X_POSITION_MINUS_7_ADDR = 0xFF4B;

/* LCD CONTROL bits */
#define LCD_CONTROL_LCD_DISPLAY_ENABLE_BIT             7 // 0 off, 1 on
#define LCD_CONTROL_WINDOW_TILE_MAP_DISPLAY_SELECT_BIT 6
#define LCD_CONTROL_WINDOW_DISPLAY_ENABLE_BIT          5 // 0 off, 1 on
#define LCD_CONTROL_BG_WINDOW_TILE_DATA_SELECT_BIT     4
#define LCD_CONTROL_BG_TILE_MAP_DISPLAY_SELECT_BIT     3
#define LCD_CONTROL_SPRITE_SIZE_BIT                    2 // 0=8x8, 1=8x16
#define LCD_CONTROL_SPRITE_DISPLAY_ENABLE_BIT          1 // 0 off, 1 on
#define LCD_CONTROL_BG_WINDOW_DISPLAY_PRIORITY_BIT     0 // 0 off, 1 on

/* LCDC Status bits */
#define LCDC_STATUS_LYC_EQUALS_LY_COINCIDENCE_INTERRUPT_BIT 6 // 1 = Enable
#define LCDC_STATUS_MODE_2_OAM_INTERRUPT_BIT                5 // 1 = Enable
#define LCDC_STATUS_MODE_1_V_BLANK_INTERRUPT_BIT            4 // 1 = Enable
#define LCDC_STATUS_MODE_0_H_BLANK_INTERRUPT_BIT            3 // 1 = Enable 
#define LCDC_STATUS_LYC_LY_COINCIDENCE_FLAG_BIT             2 // 0 if LYC < LY or LYC > LY, 1 if LYC = LY
#define LCDC_STATUS_MODE_FLAG_SECOND_BIT                    1 // 00: H_Blank mode 01: V_Blank mode
#define LCDC_STATUS_MODE_FLAG_FIRST_BIT                     0 // 10: Searching OAM mode 11: Transfer data to LCD mode

struct Pixel
{
    byte Red;
    byte Green;
    byte Blue;

    Pixel(byte i_Red, byte i_Green, byte i_Blue) : Red(i_Red), Green(i_Green), Blue(i_Blue) {}
    bool operator==(Pixel& i_Other)
    {
        return this->Red == i_Other.Red && this->Green == i_Other.Green && this->Blue == i_Other.Blue;
    }
};

const Pixel GAMEBOY_POCKET_PALLETE[4] = { { 255, 255, 255 }, { 192, 192, 192 }, { 96, 96, 96 }, { 0, 0, 0 } };

const uint32_t MAX_CYCLES_BEFORE_RENDERING = 70224; 

class Gameboy;

class GPU
{
public:
    GPU(Gameboy& i_Gameboy);
	virtual ~GPU() = default;
    GPU(const GPU&) = delete;
    GPU& operator=(const GPU&) = delete;

    void Step(uint32_t& i_Cycles);
    void Reset();

private:
    enum class Video_Mode
    {
        H_Blank = 0,
        V_Blank = 1,
        Searching_OAM = 2,
        Transfer_Data_To_LCD = 3, // access VRAM and transfer data to LCD
    };

    const uint32_t MIN_H_BLANK_MODE_CYCLES = 204; // Mode 0
    const uint32_t MIN_SEARCHING_OAM_MODE_CYCLES = 80; // Mode 2
    const uint32_t MIN_TRANSFER_DATA_TO_LCD_MODE_CYCLES = 172; // Mode 3
    const uint32_t MAX_VIDEO_CYCLES = MIN_H_BLANK_MODE_CYCLES + MIN_SEARCHING_OAM_MODE_CYCLES + MIN_TRANSFER_DATA_TO_LCD_MODE_CYCLES;
    const uint32_t V_BLANK_CYCLES = MAX_VIDEO_CYCLES * 10; // Mode 1, 10 lines 144-153

    bool isLCDEnabled();
    void setMode(Video_Mode i_NewMode);
    void handleHBlankMode();
    void handleVBlankMode();
    void handleSearchSpritesAttributesMode();
    void handleLCDTransferMode();
    void drawScanline();
    bool checkForLCDCInterrupt(int i_InterruptBit);

    Video_Mode m_Mode;
    uint32_t m_VideoCycles;

    /* gameboy ref */
    Gameboy& m_Gameboy;
};
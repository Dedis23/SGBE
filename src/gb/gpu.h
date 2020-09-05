/**********************************************************
 *		Created by: Dedi Sidi, 2020                       *
 *											              *
 *		GPU - The main graphics chip of the gameboy       *
 *********************************************************/

#pragma once
#include "gameboy.h"
#include "utility.h"
#include <cstring>

/*  Most of the info was taken from:                         *
 *  Pan Docs:                                                *
 *  https://gbdev.io/pandocs/#video-display                  *
 *  The Ultimate Game Boy Talk (33c3)                        *
 *  https://www.youtube.com/watch?v=HyzD8pNlpwI&t=           *
 *  codesligner:                                             *
 *  http://www.codeslinger.co.uk/pages/projects/gameboy.html *
 *  gameboy cpu manual pdf:                                  *
 *  http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf           */

/* gpu releated registers addresses in memory: */

/* lcd control register */
const word GPU_LCD_CONTROL_ADDR = 0xFF40;

/* lcd status register */
const word GPU_LCDC_STATUS_ADDR = 0xFF41;

/* the Y position out of 256 which the BG will start to be displayed 
   (from there 144 rows shall be displayed (wraps around if passed the edge)) */
const word GPU_SCROLL_Y_ADDR = 0xFF42;

/* the X position out of 256 which the BG will start to be displayed 
   (from there 160 cols shall be displayed (wraps around if passed the edge)) */
const word GPU_SCROLL_X_ADDR = 0xFF43;

/* current line to be drawn in the frame (out of 144 lines total) */
const word GPU_LCDC_Y_COORDINATE_ADDR = 0xFF44;

/* the value will be compared with lyc.
   if both are the same and there is a request, an interrupt will be requested */
const word GPU_LY_COMPARE_ADDR = 0xFF45;

/* writing here will launch DMA transfer process */
const word GPU_DMA_TRANSFER_AND_START_ADDR = 0xFF46; 

/* assigns shades to the color numbers out of four options (0-3) to bg and window tiles.
   every 2 bits represent a shade: 
   7-6 -> Color 3 5-4 -> Color 2 
   3-2 -> Color 1 1-0 -> Color 0 */
const word GPU_BG_AND_WINDOW_PALETTE_DATA_ADDR = 0xFF47;

/* assigns shades for sprites, palette number 0
   works like bg and window palette */
const word GPU_SPRITES_PALETTE_0_DATA_ADDR = 0xFF48; 

/* assigns shades for sprites, palette number 0
   works like bg and window palette */
const word GPU_SPRITES_PALETTE_1_DATA_ADDR = 0xFF49; 

/* the y position if the window, the window can be displayed above the background
   and sprites may be displayed above or behind the window
   the window becomes visible if it is enabled in the controller,
   the Y is in 0-143 range and the X is in 0-159 range. (windows x register is minus 7)
   for upper left the values should be Y=0, X=7 and it will be on top of the background */
const word GPU_WINDOW_Y_POSITION_ADDR = 0xFF4A;

/* the x position of the window. the details written above */
const word GPU_WINDOW_X_POSITION_MINUS_7_ADDR = 0xFF4B;

/* LCD Control bits */
#define LCD_CONTROL_LCD_DISPLAY_ENABLE_BIT             7 // 0 off, 1 on
#define LCD_CONTROL_WINDOW_TILE_MAP_INDEX_SELECT_BIT   6 // 0 = take tile index from 0x9800-0x9BFF, 1 = take tile index from 0x9c00-0x9FFF
#define LCD_CONTROL_WINDOW_DISPLAY_ENABLE_BIT          5 // 0 off, 1 on
#define LCD_CONTROL_BG_AND_WINDOW_TILE_DATA_SELECT_BIT 4 // 0 = take tile data from 0x8800-0x97FF, 1 = take tile data from 0x8000-0x8FFF
#define LCD_CONTROL_BG_TILE_MAP_INDEX_SELECT_BIT       3 // 0 = take tile index from 0x9800-0x9BFF, 1 = take tile index from 0x9c00-0x9FFF
#define LCD_CONTROL_SPRITE_SIZE_BIT                    2 // 0 is 8x8 sprite size, 1 is 8x16 sprite size
#define LCD_CONTROL_SPRITE_DISPLAY_ENABLE_BIT          1 // 0 off, 1 on
#define LCD_CONTROL_BG_WINDOW_DISPLAY_PRIORITY_BIT     0 // 0 = window and bg off, 1 on, in CGB mode, there more to do here with priority

/* LCD Status bits */
#define LCD_STATUS_LYC_EQUALS_LY_COINCIDENCE_INTERRUPT_BIT 6 // 1 = Enable
#define LCD_STATUS_MODE_2_OAM_INTERRUPT_BIT                5 // 1 = Enable
#define LCD_STATUS_MODE_1_V_BLANK_INTERRUPT_BIT            4 // 1 = Enable
#define LCD_STATUS_MODE_0_H_BLANK_INTERRUPT_BIT            3 // 1 = Enable 
#define LCD_STATUS_LYC_LY_COINCIDENCE_FLAG_BIT             2 // 0 if LYC < LY or LYC > LY, 1 if LYC = LY
#define LCD_STATUS_MODE_FLAG_SECOND_BIT                    1 // 00: H_Blank mode 01: V_Blank mode
#define LCD_STATUS_MODE_FLAG_FIRST_BIT                     0 // 10: Searching OAM mode 11: Transfer data to LCD mode

/* Various definitions */

/* mode and cycles releated */
#define MIN_H_BLANK_MODE_CYCLES              204   // Mode 0 cycles
#define MIN_V_BLANK_MODE_CYCLES              456   // Mode 1 cycles (for 1 line)
#define MAX_V_BLANK_MODE_CYCLES              4560  // Mode 1 overall cycles (10 lines) 
#define MIN_SEARCHING_OAM_MODE_CYCLES        80    // Mode 2 cycles
#define MIN_TRANSFER_DATA_TO_LCD_MODE_CYCLES 172   // Mode 3 cycles
#define MAX_CYCLES_BEFORE_RENDERING          70224 // this is calcualted like so:  144 lines in modes 2, 3, 0 (144*456 cycles)
                                                   // plus 10 lines in mode 1 (10*456) = 65664 + 4560 = 70224
                                                   // 456 is the number of cycles that it takes to draw single line (mode 2 + mode3 + mode0)
#define V_BLANK_START_SCANLINE 144
#define V_BLANK_END_SCANLINE 154 // lines 144 till 153 are for vblank mode (10 lines)

/* background, window and sprites releated (tiles, pixels) */
#define BG_HEIGHT_PIXELS 256 // the background consists of 256*256 or 32*32 tiles
#define BG_WIDTH_PIXELS 256
#define MAX_TILES_PER_LINE 32
#define TILE_HEIGHT_IN_PIXELS 8
#define TILE_WIDTH_IN_PIXELS  8
#define SIZE_OF_A_SINGLE_TILE_IN_BYTES 16
#define SIZE_OF_A_SINGLE_LINE_IN_A_TILE_IN_BYTES 2
#define WINDOW_TILE_MAP_ADDR_IF_BIT_IS_0 0x9800
#define WINDOW_TILE_MAP_ADDR_IF_BIT_IS_1 0x9C00
#define BG_AND_WINDOW_TILE_DATA_ADDR_IF_BIT_IS_0 0x8800
#define BG_AND_WINDOW_TILE_DATA_ADDR_IF_BIT_IS_1 0x8000
#define BG_AND_WINDOW_TILE_MAP_ADDR_IF_BIT_IS_0 0x9800
#define BG_AND_WINDOW_TILE_MAP_ADDR_IF_BIT_IS_1 0x9C00
#define WINODW_Y_MAX_ROW 143
#define WINDOW_X_MAX_COL 159

/* sprites releated */
struct Sprite
{
    byte PositionY; // for y = 0, the PositionY is 16 in memory
    byte PositionX; // for x = 0, the PositionX is 8 in memory
    byte TileIndex;
    byte Attributes;
};
#define SPRITE_ATTR_SPRITE_TO_BG_AND_WINDOW_PRIORITY_BIT 7 // 0 = sprite is above BG and window, 1 = the sprite is behind the background and window, 
                                                           // unless the color of them is white (brightest in palette) 
                                                           // and then the sprite is rendered still on top of them
#define SPRITE_ATTR_Y_FLIP_BIT                           6 // 0 = normal, 1 = vertically mirrored
#define SPRITE_ATTR_X_FLIP_BIT                           5 // 0 = normal, 1 = horizontally mirrored
#define SPRITE_ATTR_PALLETE_NUMBER_FOR_NON_CGB_BIT       4 // 0 = use sprite pallete #0, 1 = use sprite pallete #1 
#define SPRITE_ATTR_TILE_VRAM_BANK_BIT                   3 // 0 = bank 0, 1 = bank 1 - Note this is for CGB only.
#define SPRITE_ATTR_PALLETE_NUMBER_FOR_CGB_THIRD_BIT     2 // *
#define SPRITE_ATTR_PALLETE_NUMBER_FOR_CGB_SECOND_BIT    1 // bits 2-0 are for palette number 0-7. thse are used in CGB mode only.
#define SPRITE_ATTR_PALLETE_NUMBER_FOR_CGB_FIRST_BIT     0 // *
#define SPRITE_TILE_DATA_BASE_ADDR 0x8000
#define SPRITES_BASE_ADDR 0xFE00 // (start of OAM)
#define NUM_OF_SPRITES_ENTRIES 40
#define SIZE_OF_SPRITE_DATA 4

/* pixel releated */
struct Pixel
{
    byte Red;
    byte Green;
    byte Blue;

    Pixel() : Red(255), Green(255), Blue(255) {}
    Pixel(byte i_Red, byte i_Green, byte i_Blue) : Red(i_Red), Green(i_Green), Blue(i_Blue) {}
    bool operator==(const Pixel& i_Other) const
    {
        return this->Red == i_Other.Red && this->Green == i_Other.Green && this->Blue == i_Other.Blue;
    }
};

/* palettes */
enum class Palette
{
    Pocket_Pallete, /* Gameboy pocket palette (Grey Shades) */
    Original_Pallete, /* Original Gameboy palette (Green Shades) */
    Autmn_Pallete /* Autmn palette (custom) */
};

#define NUM_OF_PALETTES 3
#define NUM_OF_SHADES_IN_PALETTE 4
const Pixel Palettes[NUM_OF_PALETTES][NUM_OF_SHADES_IN_PALETTE] = 
                                                            { 
                                                                { { 255, 255, 255 }, { 192, 192, 192 }, { 96, 96, 96 }, { 0, 0, 0 } }, /* GB Pocket */
                                                                { { 155, 188, 15 }, { 139, 172, 15 }, { 49, 98, 48 }, { 15, 56, 15 } }, /* Original GB */
                                                                { { 255, 246, 211 }, { 249, 168, 117 }, { 235, 107, 111 }, { 124, 63, 88 } } /* Autmn */
                                                            };

class GBInternals;

class GPU
{
public:
    GPU(GBInternals& i_GBInternals);
	virtual ~GPU() = default;
    GPU(const GPU&) = delete;
    GPU& operator=(const GPU&) = delete;

    void Step(bool write, std::ostream& os, const uint32_t& i_Cycles); // TODO remove this ostream from here, (its for debug)
    void Reset();
    const Pixel* GetFrameBuffer() const;
    byte GetRegister(word i_Address) const;
    void SetRegister(word i_Address, byte i_Value);
    void ChangePalette(const Palette& i_Palette);
    void ChangeToNextPalette();

private:
    enum class Video_Mode
    {
        H_Blank = 0,
        V_Blank = 1,
        Searching_OAM = 2,
        Transfer_Data_To_LCD = 3, // access VRAM and transfer data to LCD
    };

    enum class Shade
    {
        Shade_00 = 0, // Brightest
        Shade_01 = 1,
        Shade_10 = 2,
        Shade_11 = 3, // Darkest
    };

    void setMode(Video_Mode i_NewMode);
    void handleHBlankMode();
    void handleVBlankMode();
    void handleSearchSpritesAttributesMode();
    void handleLCDTransferMode();
    void checkForLYAndLYCCoincidence();

    void drawCurrentScanline();
    void drawCurrentLineBackground();
    void drawCurrentLineWindow();
    void drawCurrentLineSprites();
    void readTileLineFromMemory(const uint32_t& i_XPosition, const uint32_t& i_YPosition,
                                word i_TileDataBaseAddr, word i_TileIndexMapBaseAddr, bool i_IsSignedDataRegion,
                                byte& o_HighByte, byte& o_LowByte);
    Shade extractShadeIdFromTileLine(byte i_HighByte, byte i_LowByte, byte i_TilePixelCol);
    Shade extractRealShadeFromPalette(byte i_Palette, Shade i_ShadeId);

    bool m_IsLCDEnabled;
    Video_Mode m_Mode;
    uint32_t m_VideoCycles;
    Pixel m_FrameBuffer[GAMEBOY_SCREEN_WIDTH * GAMEBOY_SCREEN_HEIGHT];
    Pixel* m_PalettePtr;
    Palette m_CurrPalette;
    byte m_LCDControl;
    byte m_LCDStatus;
    byte m_ScrollY;
    byte m_ScrollX;
    byte m_LCDCYCoordinate;
    byte m_LYCompare;
    byte m_BGAndWindowPalette;
    byte m_SpritesPalette0;
    byte m_SpritesPalette1;
    byte m_WindowYPosition;
    byte m_WindowXPositionMinus7;

	/* gameboy components ref */
	GBInternals& m_GBInternals;
};
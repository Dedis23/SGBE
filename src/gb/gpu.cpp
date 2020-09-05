#include "gpu.h"

GPU::GPU(GBInternals& i_GBInternals) : m_GBInternals(i_GBInternals), m_IsLCDEnabled(true), m_Mode(Video_Mode::Searching_OAM), m_VideoCycles(0),
m_LCDControl(0), m_LCDStatus(0), m_ScrollY(0), m_ScrollX(0), m_LCDCYCoordinate(0), m_LYCompare(0), m_BGAndWindowPalette(0),
m_SpritesPalette0(0), m_SpritesPalette1(0), m_WindowYPosition(0), m_WindowXPositionMinus7(0)
{
	ChangePalette(Palette::Pocket_Pallete);
}

/*
	Taken from Pan Docs:
	The following are typical when the display is enabled:
	Mode 2  2_____2_____2_____2_____2_____2___________________2____
	Mode 3  _33____33____33____33____33____33__________________3___
	Mode 0  ___000___000___000___000___000___000________________000
	Mode 1  ____________________________________11111111111111_____

	the gpu switches between the modes in the way above.
	modes 2 (Search OAM), 3 (transfer data to LCD), 0 (HBlank) untill 144 of the lines have been drawn (sent to frame buffer)
	then mode 1 (VBlank) happens for the next 10 lines 144-153
	all of this happens per frame and in total it takes 70224 cycles
*/
void GPU::Step(bool write, std::ostream& os, const uint32_t& i_Cycles)
{
	// synchronization explained:
	// in order to synchronize between the CPU and GPU in the emulation,
	// each step we update the member m_VideoCycles by the cycles that the cpu did in the last step
	// only once the cpu have made enough (the minimum) cycles that correspond to the current mode, then we actualy do the mode operation
	if (m_IsLCDEnabled)
	{
		if (write) // TODO remove this part (just for debug)
		{
			//os << "GPU Registers dump:" << std::endl;
			//os << "m_LCDControl; " << std::dec << (int)m_LCDControl << std::endl;
			//os << "m_LCDStatus; " << std::dec << (int)m_LCDStatus << std::endl;
			//os << "m_ScrollY; " << std::dec << (int)m_ScrollY << std::endl;
			//os << "m_ScrollX; " << (int)m_ScrollX << std::endl;
			//os << "m_LCDCYCoordinate; " << std::dec << (int)m_LCDCYCoordinate << std::endl;
			//os << "m_LYCompare; " << std::dec << (int)m_LYCompare << std::endl;
			//os << "m_BGPaletteData; " << std::dec << (int)m_BGAndWindowPalette << std::endl;
			//os << "m_ObjectPalette0; " << std::dec << (int)m_SpritesPalette0 << std::endl;
			//os << "m_ObjectPalette1; " << std::dec << (int)m_SpritesPalette1 << std::endl;
			//os << "m_WindowYPosition; " << std::dec << (int)m_WindowYPosition << std::endl;
			//os << "m_WindowXPositionMinus7; " << std::dec << (int)m_WindowXPositionMinus7 << std::endl;
		}
		if (write)
		{
			//os << "video cycles before: " << std::dec << (int)m_VideoCycles << std::endl;
		}
		m_VideoCycles += i_Cycles;
		if (write)
		{
			//os << "video cycles after: " << std::dec << (int)m_VideoCycles << std::endl;
		}
		switch (m_Mode)
		{
		case GPU::Video_Mode::H_Blank:
			handleHBlankMode();
			break;
		case GPU::Video_Mode::V_Blank:
			handleVBlankMode();
			break;
		case GPU::Video_Mode::Searching_OAM:
			handleSearchSpritesAttributesMode();
			break;
		case GPU::Video_Mode::Transfer_Data_To_LCD:
			handleLCDTransferMode();
			break;
		}
	}
}

void GPU::Reset()
{
	m_LCDControl = 0;
	m_LCDStatus = 0;
	m_ScrollY = 0;
	m_ScrollX = 0;
	m_LCDCYCoordinate = 0;
	m_LYCompare = 0;
	m_BGAndWindowPalette = 0;
	m_SpritesPalette0 = 0;
	m_SpritesPalette1 = 0;
	m_WindowYPosition = 0;
	m_WindowXPositionMinus7 = 0;
	m_IsLCDEnabled = true;
	m_VideoCycles = 0;
	memset(m_FrameBuffer, 0xFF, sizeof(m_FrameBuffer));	// reset all pixels to white 255 in RGB
	setMode(Video_Mode::Searching_OAM);
}

const Pixel* GPU::GetFrameBuffer() const
{
	return m_FrameBuffer;
}

byte GPU::GetRegister(word i_Address) const
{
	switch (i_Address)
	{
	case GPU_LCD_CONTROL_ADDR:
		{
			return m_LCDControl;
		}
		break;
	case GPU_LCDC_STATUS_ADDR:
		{
			return m_LCDStatus;
		}
		break;
	case GPU_SCROLL_Y_ADDR:
		{
			return m_ScrollY;
		}
		break;
	case GPU_SCROLL_X_ADDR:
		{
			return m_ScrollX;
		}
		break;
	case GPU_LCDC_Y_COORDINATE_ADDR:
		{
			return m_LCDCYCoordinate;
		}
		break;
	case GPU_LY_COMPARE_ADDR:
		{
			return m_LYCompare;
		}
		break;
	case GPU_BG_AND_WINDOW_PALETTE_DATA_ADDR:
		{
			return m_BGAndWindowPalette;
		}
		break;
	case GPU_SPRITES_PALETTE_0_DATA_ADDR:
		{
			return m_SpritesPalette0;
		}
		break;
	case GPU_SPRITES_PALETTE_1_DATA_ADDR:
		{
			return m_SpritesPalette1;
		}
		break;
	case GPU_WINDOW_Y_POSITION_ADDR:
		{
			return m_WindowYPosition;
		}
		break;
	case GPU_WINDOW_X_POSITION_MINUS_7_ADDR:
		{
			return m_WindowXPositionMinus7;
		}
		break;
	}

	LOG_ERROR(true, return 0, "Attempting to read from unmapped memory address: 0x" << i_Address);
}

void GPU::SetRegister(word i_Address, byte i_Value)
{
	switch (i_Address)
	{
	case GPU_LCD_CONTROL_ADDR:
		{
			m_LCDControl = i_Value;
		}
		break;
	case GPU_LCDC_STATUS_ADDR:
		{
			// bits 0-2 should not be written by the game they are read only (2 is LY~LYC coincidence bit and 0-1 are the current mode)
			i_Value &= 0xF8;
			m_LCDStatus &= 0x07;
			m_LCDStatus |= i_Value;
		}
		break;
	case GPU_SCROLL_Y_ADDR:
		{
			m_ScrollY = i_Value;
		}
		break;
	case GPU_SCROLL_X_ADDR:
		{
			m_ScrollX = i_Value;
		}
		break;
	case GPU_LCDC_Y_COORDINATE_ADDR:
		{
			// whenever the user write to the Y coordinate, it will reset
			m_LCDCYCoordinate = 0;
		}
		break;
	case GPU_LY_COMPARE_ADDR:
		{
			m_LYCompare = i_Value;
		}
		break;
	case GPU_DMA_TRANSFER_AND_START_ADDR:
		{
			m_GBInternals.GetMMU().DMATransfer(i_Value);
		}
		break;
	case GPU_BG_AND_WINDOW_PALETTE_DATA_ADDR:
		{
			m_BGAndWindowPalette = i_Value;
		}
		break;
	case GPU_SPRITES_PALETTE_0_DATA_ADDR:
		{
			m_SpritesPalette0 = i_Value;
		}
		break;
	case GPU_SPRITES_PALETTE_1_DATA_ADDR:
		{
			m_SpritesPalette1 = i_Value;
		}
		break;
	case GPU_WINDOW_Y_POSITION_ADDR:
		{
			m_WindowYPosition = i_Value;
		}
		break;
	case GPU_WINDOW_X_POSITION_MINUS_7_ADDR:
		{
			m_WindowXPositionMinus7 = i_Value;
		}
		break;
	}
}

/* mode 0 handler */
void GPU::handleHBlankMode()
{
	if (m_VideoCycles >= MIN_H_BLANK_MODE_CYCLES)
	{
		m_VideoCycles -= MIN_H_BLANK_MODE_CYCLES;

		// increment the y scanline
		m_LCDCYCoordinate++;

		// after a change in the scanline, check for LY and LYC coincidence interrupt
		checkForLYAndLYCCoincidence();

		// move either to mode 2 or mode 1, based on current scanline
		if (m_LCDCYCoordinate == V_BLANK_START_SCANLINE)
		{
			// request vblank interrupt
			m_GBInternals.GetCPU().RequestInterrupt(CPU::InterruptType::VBlank);

			// check for mode 1 interrupt bit
			if (bitwise::IsBitSet(LCD_STATUS_MODE_1_V_BLANK_INTERRUPT_BIT, m_LCDStatus))
			{
				m_GBInternals.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
			}

			// move to mode 1
			setMode(Video_Mode::V_Blank);
		}
		else // move again to OAM search (mode 2) to create the next scanline
		{
			// check for mode 2 interrupt bit
			if (bitwise::IsBitSet(LCD_STATUS_MODE_2_OAM_INTERRUPT_BIT, m_LCDStatus))
			{
				m_GBInternals.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
			}

			// move to mode 2
			setMode(Video_Mode::Searching_OAM);
		}
	}
}

/* mode 1 handler */
void GPU::handleVBlankMode()
{
	if (m_VideoCycles >= MIN_V_BLANK_MODE_CYCLES)
	{
		m_VideoCycles -= MIN_V_BLANK_MODE_CYCLES;
		// increment the y corrdinate
		m_LCDCYCoordinate++;
		// after a change in the scanline, check for LY and LYC coincidence interrupt
		checkForLYAndLYCCoincidence();
	}
	
	// if its the end of vblank mode
	if (m_LCDCYCoordinate > V_BLANK_END_SCANLINE)
	{
		m_VideoCycles = 0;
		// reset y coordinate
		m_LCDCYCoordinate = 0;
		// after a change in the scanline, check for LY and LYC coincidence interrupt
		checkForLYAndLYCCoincidence();
	
		// check for mode 2 interrupt bit
		if (bitwise::IsBitSet(LCD_STATUS_MODE_2_OAM_INTERRUPT_BIT, m_LCDStatus))
		{
			m_GBInternals.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
		}
	
		// move to mode 2
		setMode(Video_Mode::Searching_OAM);
	}
}

/* mode 2 handler */
void GPU::handleSearchSpritesAttributesMode()
{
	if (m_VideoCycles >= MIN_SEARCHING_OAM_MODE_CYCLES)
	{
		m_VideoCycles -= MIN_SEARCHING_OAM_MODE_CYCLES;

		// move to mode 3 (Transfer data to LCD)
		setMode(Video_Mode::Transfer_Data_To_LCD);
	}
}

/* mode 3 handler */
void GPU::handleLCDTransferMode()
{
	if (m_VideoCycles >= MIN_TRANSFER_DATA_TO_LCD_MODE_CYCLES)
	{
		m_VideoCycles -= MIN_TRANSFER_DATA_TO_LCD_MODE_CYCLES;

		// write a single scanline
		drawCurrentScanline();

		// check for mode 0 (H Blank) interrupt bit
		if (bitwise::IsBitSet(LCD_STATUS_MODE_0_H_BLANK_INTERRUPT_BIT, m_LCDStatus))
		{
			m_GBInternals.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
		}

		// move to mode 0 (H Blank)
		setMode(Video_Mode::H_Blank);
	}
}

void GPU::checkForLYAndLYCCoincidence()
{
	// compare the current Y scanline and LYC
	if (m_LCDCYCoordinate == m_LYCompare)
	{
		// raise coincidence bit in the status register
		bitwise::SetBit(LCD_STATUS_LYC_LY_COINCIDENCE_FLAG_BIT, m_LCDStatus);
		// request interrupt if the coincidence interrupt bit is raised
		if (bitwise::IsBitSet(LCD_STATUS_LYC_EQUALS_LY_COINCIDENCE_INTERRUPT_BIT, m_LCDStatus))
		{
			m_GBInternals.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
		}
	}
	else //  its > or <
	{
		// clear the coincidence bit in the status register
		bitwise::ClearBit(LCD_STATUS_LYC_LY_COINCIDENCE_FLAG_BIT, m_LCDStatus);
	}
}

/* draws a single scanline with background, window and sprite */
void GPU::drawCurrentScanline()
{
	// check for BG/window display bit
	if (bitwise::IsBitSet(LCD_CONTROL_BG_WINDOW_DISPLAY_PRIORITY_BIT, m_LCDControl))
	{
		drawCurrentLineBackground();
		// check for window display bit
		if (bitwise::IsBitSet(LCD_CONTROL_WINDOW_DISPLAY_ENABLE_BIT, m_LCDControl))
		{
			drawCurrentLineWindow();
		}
	}
	// check for sprite display bit
	if (bitwise::IsBitSet(LCD_CONTROL_SPRITE_DISPLAY_ENABLE_BIT, m_LCDControl))
	{
		drawCurrentLineSprites();
	}
}

/* draw a single line of the background
   the background consists of 256*256 pixels. by using the SCROLL_Y and SCROLL_X registers,
   the game decides from where in the 256*256 map, it should draw (this wraps around if passes the edge) */
void GPU::drawCurrentLineBackground()
{
	word tileIndexMapBaseAddr = 0x0;
	word tileDataBaseAddr = 0x0;
	bool isSignedTileDataRegion = false;

	// check which background memory section is relevant
	if (bitwise::IsBitSet(LCD_CONTROL_BG_TILE_MAP_INDEX_SELECT_BIT, m_LCDControl))
	{
		tileIndexMapBaseAddr = BG_AND_WINDOW_TILE_MAP_ADDR_IF_BIT_IS_1;
	}
	else
	{
		tileIndexMapBaseAddr = BG_AND_WINDOW_TILE_MAP_ADDR_IF_BIT_IS_0;
	}	

	// check which tile data is relevant
	if (bitwise::IsBitSet(LCD_CONTROL_BG_AND_WINDOW_TILE_DATA_SELECT_BIT, m_LCDControl))
	{
		tileDataBaseAddr = BG_AND_WINDOW_TILE_DATA_ADDR_IF_BIT_IS_1;
		isSignedTileDataRegion = false;
	}
	else
	{
		tileDataBaseAddr = BG_AND_WINDOW_TILE_DATA_ADDR_IF_BIT_IS_0;
		isSignedTileDataRegion = true; // this memory region uses signed bytes as tile id's
	}
	
	/* calculate the pixel y position, this is the same for every X pixel because we draw a single line */
	uint32_t yPos = (m_LCDCYCoordinate + m_ScrollY) % BG_HEIGHT_PIXELS;

	// for every pixel in the current line do the follwing
	for (uint32_t xIndex = 0; xIndex < GAMEBOY_SCREEN_WIDTH; xIndex++)
	{
		// calculate the current pixel x position
		// (wraps around if it passed the edge of the BG)
		uint32_t xPos = (xIndex + m_ScrollX) % BG_WIDTH_PIXELS;

		byte highByte = 0x0;
		byte lowByte = 0x0;
		// read the two bytes that represent the line within the relevant tile
		readTileLineFromMemory(xPos, yPos, tileDataBaseAddr, tileIndexMapBaseAddr, isSignedTileDataRegion, highByte, lowByte);

		// calculate the pixel col number within the tile (0-7)
		uint32_t tilePixelCol = xPos % TILE_WIDTH_IN_PIXELS;
		
		// extract the shade id of the pixel from the tile line bytes
		Shade shadeId = extractShadeIdFromTileLine(highByte, lowByte, tilePixelCol);

		// translate the shade id of the pixel into the real shade based on the BG palette
		Shade realShade = extractRealShadeFromPalette(m_BGAndWindowPalette, shadeId);

		// calculate the index in the frame buffer (the array of pixels to be drawn)
		uint32_t frameBufferIndex = (m_LCDCYCoordinate * GAMEBOY_SCREEN_WIDTH) + xIndex;

		// get the pixel RGB colors to be drawn from the current palette based on the real shade number
		Pixel color = m_PalettePtr[(int)realShade];

		m_FrameBuffer[frameBufferIndex] = color;
	}
}

/* the window is behind the sprites and above the background (unless specified otherwise)
   it is generally used for UI and its a fixed panel that will generaly not scroll */
void GPU::drawCurrentLineWindow()
{
	word tileIndexMapBaseAddr = 0x0;
	word tileDataBaseAddr = 0x0;
	bool isSignedTileDataRegion = false;

	// check which window memory section is relevant
	if (bitwise::IsBitSet(LCD_CONTROL_WINDOW_TILE_MAP_INDEX_SELECT_BIT, m_LCDControl))
	{
		tileIndexMapBaseAddr = BG_AND_WINDOW_TILE_MAP_ADDR_IF_BIT_IS_1;
	}
	else
	{
		tileIndexMapBaseAddr = BG_AND_WINDOW_TILE_MAP_ADDR_IF_BIT_IS_0;
	}	

	// check which tile data is relevant
	if (bitwise::IsBitSet(LCD_CONTROL_BG_AND_WINDOW_TILE_DATA_SELECT_BIT, m_LCDControl))
	{
		tileDataBaseAddr = BG_AND_WINDOW_TILE_DATA_ADDR_IF_BIT_IS_1;
	}
	else
	{
		tileDataBaseAddr = BG_AND_WINDOW_TILE_DATA_ADDR_IF_BIT_IS_0;
		isSignedTileDataRegion = true; // this memory region uses signed bytes as tile id's
	}
	
	/* calculate the pixel y position, tile row number and pixel row number within the tile
	   all of these are the same for every X pixel because we draw a single line */
	uint32_t yPos = m_LCDCYCoordinate - m_WindowYPosition;
	if (yPos > WINODW_Y_MAX_ROW || yPos > m_LCDCYCoordinate) // if were out of range, return
		return;

	// for every pixel in the current line do the follwing
	for (uint32_t xIndex = 0; xIndex < GAMEBOY_SCREEN_WIDTH; xIndex++)
	{
		// calculate the current pixel x position
		uint32_t xPos = xIndex + m_WindowXPositionMinus7 - 7;
		
		byte highByte = 0x0;
		byte lowByte = 0x0;
		// read the two bytes that represent the line within the relevant tile
		readTileLineFromMemory(xPos, yPos, tileDataBaseAddr, tileIndexMapBaseAddr, isSignedTileDataRegion, highByte, lowByte);

		// calculate the pixel col number within the tile (0-7)
		uint32_t tilePixelCol = xPos % TILE_WIDTH_IN_PIXELS;

		// extract the shade id of the pixel from the tile line bytes
		Shade shadeId = extractShadeIdFromTileLine(highByte, lowByte, tilePixelCol);

		// translate the shade id of the pixel into the real shade based on the BG palette
		Shade realShade = extractRealShadeFromPalette(m_BGAndWindowPalette, shadeId);

		// calculate the index in the frame buffer (the array of pixels to be drawn)
		uint32_t frameBufferIndex = (m_LCDCYCoordinate * GAMEBOY_SCREEN_WIDTH) + xIndex;

		// get the real color to be drawn from the current palette based on the real shade number
		Pixel color = m_PalettePtr[(int)realShade];

		m_FrameBuffer[frameBufferIndex] = color;
	}
}

void GPU::drawCurrentLineSprites()
{
	// check in which mode the sprites are: 8x8 or 8x16
	byte spriteHeight = bitwise::IsBitSet(LCD_CONTROL_SPRITE_SIZE_BIT, m_LCDControl) ? 16 : 8;

	for (int spriteIndex = 0; spriteIndex < NUM_OF_SPRITES_ENTRIES; spriteIndex++)
	{
		// calculate the current sprite index in memory
		word spriteAddress = SPRITES_BASE_ADDR + (spriteIndex * SIZE_OF_SPRITE_DATA);

		// read the current sprite data from memory
		Sprite currSprite;
		currSprite.PositionY = m_GBInternals.GetMMU().Read(spriteAddress);
		currSprite.PositionX = m_GBInternals.GetMMU().Read(spriteAddress + 1);
		currSprite.TileIndex = m_GBInternals.GetMMU().Read(spriteAddress + 2);
		currSprite.Attributes = m_GBInternals.GetMMU().Read(spriteAddress + 3);

		// adjust y and x pos to screen pos - for (0,0) (top-left) the sprite is (8,16)
		currSprite.PositionY -= 16;
		currSprite.PositionX -= 8;

		// check if the current sprite y pos is relevant for the current scanline
		// if the position is above the curr scanline or that the position y + size does not axis intersect with it, continue 
		if (currSprite.PositionY > m_LCDCYCoordinate || currSprite.PositionY + spriteHeight <= m_LCDCYCoordinate)
			continue;
		
		// calculate the row num within the tile (0-7 / 0-15 based on sprite size) of the current sprite that intersect with the current scanline
		int tileRow = m_LCDCYCoordinate - currSprite.PositionY;

		// handle y flip
		if (bitwise::IsBitSet(SPRITE_ATTR_Y_FLIP_BIT, currSprite.Attributes))
		{
			// y flip bit is on - the trick is to read the sprite backwards from memory
			// i.e: for sprite size = 8, instead of reading line 0, will read line 7, 1->6 and so on
			tileRow -= (spriteHeight - 1);
		}

		// read the two bytes that represent the line of intersection
		word tileDataAddr = SPRITE_TILE_DATA_BASE_ADDR + (currSprite.TileIndex * SIZE_OF_A_SINGLE_TILE_IN_BYTES);
		word addressForTheRowInTheTile = tileDataAddr + (tileRow * SIZE_OF_A_SINGLE_LINE_IN_A_TILE_IN_BYTES);
		byte highByte = m_GBInternals.GetMMU().Read(addressForTheRowInTheTile);
		byte lowByte = m_GBInternals.GetMMU().Read(addressForTheRowInTheTile + 1);

		bool xFlip = bitwise::IsBitSet(SPRITE_ATTR_X_FLIP_BIT, currSprite.Attributes);
		byte palette = bitwise::IsBitSet(SPRITE_ATTR_PALLETE_NUMBER_FOR_NON_CGB_BIT, currSprite.Attributes) ? m_SpritesPalette1 : m_SpritesPalette0;
		bool isSpriteOnTop = bitwise::IsBitSet(SPRITE_ATTR_SPRITE_TO_BG_AND_WINDOW_PRIORITY_BIT, currSprite.Attributes) ? false : true;
		Pixel shade0RGB = m_PalettePtr[(int)extractRealShadeFromPalette(m_BGAndWindowPalette, Shade::Shade_00)];

		// for every one of the 8 pixels in the row, do the following
		for (int xIndex = 0; xIndex < 8; xIndex++)
		{
			int xPos = currSprite.PositionX + xIndex;

			// continue only if the pixel is on screen
			if (xPos < 0 || xPos >= GAMEBOY_SCREEN_WIDTH)
				continue;

			// handle x flip
			byte colInLine = xIndex;
			if (xFlip)
			{
				// x flip bit is on - read the pixel backwards
				// i.e: instead of reading pixel 0, read pixel 7, 1->6 and so on
				colInLine = 7 - xIndex;
			}	

			// extract the shade id of the pixel from the tile line bytes
			Shade shadeId = extractShadeIdFromTileLine(highByte, lowByte, colInLine);

			// if the sprite shade id is 0x0 then its transperent, do nothing
			if (shadeId == Shade::Shade_00)
				continue;

			// calculate the index in the frame buffer (the array of pixels to be drawn)
			uint32_t frameBufferIndex = (m_LCDCYCoordinate * GAMEBOY_SCREEN_WIDTH) + xPos;

			// now continue only if the sprite on top flag is on or, 
			// the current pixel in the background is white (0x0 - brightest shade)
			// in this case, the flag is ignored and the sprite shall be displayed nontheless
			if (isSpriteOnTop || m_FrameBuffer[frameBufferIndex] == shade0RGB)
			{
				// translate the shade id of the pixel into the real shade based on the current sprite palette
				Shade realShade = extractRealShadeFromPalette(palette, shadeId);

				// get the real color to be drawn from the current palette based on the real shade number
				Pixel color = m_PalettePtr[(int)realShade];

				m_FrameBuffer[frameBufferIndex] = color;
			}
		}
	}
}

/* used for BG and window tiles drawings */
inline void GPU::readTileLineFromMemory(const uint32_t& i_XPosition, const uint32_t& i_YPosition,
                                        word i_TileDataBaseAddr, word i_TileIndexMapBaseAddr, bool i_IsSignedDataRegion,
                                        byte& o_HighByte, byte& o_LowByte)
{
	// calculate the tile number that this pixel corresponds to (0-31)
	uint32_t tileCol = i_XPosition / TILE_WIDTH_IN_PIXELS;
	uint32_t tileRow = i_YPosition / TILE_HEIGHT_IN_PIXELS;

	// calculate the pixel row number within the tile (0-7)
	uint32_t tilePixelRow = i_YPosition % TILE_HEIGHT_IN_PIXELS;

	// calculate tile index in map
	word tileIndexInMap = i_TileIndexMapBaseAddr + (tileRow * MAX_TILES_PER_LINE + tileCol);
	
	// get the tile id from memory
	byte tileId = m_GBInternals.GetMMU().Read(tileIndexInMap);

	// calculate the tile data address 
	// the id that we mapped should be added to the tile data address base
	// this way we know which tile data to use for this pixel
	// note that we need to take into account the sign in this region
	word tileDataAddr = i_TileDataBaseAddr;
	if (i_IsSignedDataRegion)
	{
		tileDataAddr += (static_cast<sbyte>(tileId) + 128) * SIZE_OF_A_SINGLE_TILE_IN_BYTES;
	}
	else
	{
		tileDataAddr += tileId * SIZE_OF_A_SINGLE_TILE_IN_BYTES;
	}

	// now that we have the exact tile data id in memory,
	// we need to know which out of the 8 lines in the tile,
	// we need to read (remember that 2 bytes = 1 line)
	// we already caluclated the row in which the pixel is on the tile
	// just need this number multiplied (because its 2 bytes)
	// and read that from memory
	word addressForTheRowInTheTile = tileDataAddr + (tilePixelRow * SIZE_OF_A_SINGLE_LINE_IN_A_TILE_IN_BYTES);

	// read the two bytes of this line from memory
	o_HighByte = m_GBInternals.GetMMU().Read(addressForTheRowInTheTile);
	o_LowByte = m_GBInternals.GetMMU().Read(addressForTheRowInTheTile + 1);
}

inline GPU::Shade GPU::extractShadeIdFromTileLine(byte i_HighByte, byte i_LowByte, byte i_TilePixelCol)
{
	// the ids are read if we put the bytes on top of each other like that:
	// pixel 		0 1 2 3 4 5 6 7
	// lower byte   a b c d e f g h
	// upper byte   i j k l m n o p
	// so for pixel 0 we need to extract the first two bits and it will be the id "ai"
	// note that pixel 0 is actually bits 7 and pixel 7 is bits 0

	byte lowerBit = (int)bitwise::IsBitSet(7 - i_TilePixelCol, i_LowByte);
	byte upperBit = (int)bitwise::IsBitSet(7 - i_TilePixelCol, i_HighByte);

	byte shadeId = (lowerBit << 1) | upperBit;
	return Shade(shadeId);
}

inline GPU::Shade GPU::extractRealShadeFromPalette(byte i_Palette, Shade i_ShadeId)
{
	// every two bits in the palette represent a shade
	// it maps like so:
	// bits 1-0 to shade id 00
	// bits 3-2 to shade id 01
	// bits 5-4 to shade id 10
	// bits 7-6 to shade id 11
	// what is written within the bits in the palette is the real shade to be drawn

	byte realShadeToDraw = 0x0;
	switch (i_ShadeId)
	{
		case GPU::Shade::Shade_00:
			// extract bits 1-0 from palette
			realShadeToDraw = i_Palette & 0x03;
			break;
		case GPU::Shade::Shade_01:
			// extract bits 3-2 from palette
			realShadeToDraw = (i_Palette & 0x0C) >> 2;
			break;
		case GPU::Shade::Shade_10:
			// extract bits 5-4 from palette
			realShadeToDraw = (i_Palette & 0x30) >> 4;
			break;
		case GPU::Shade::Shade_11:
			// extract bits 7-6 from palette
			realShadeToDraw = (i_Palette & 0xC0) >> 6;
			break;
		default:
			break;
	}

	return Shade(realShadeToDraw);
}

void GPU::setMode(Video_Mode i_NewMode)
{
	// set bits 0 and 1 in status LCD register to the new mode
	switch (i_NewMode)
	{
	case GPU::Video_Mode::H_Blank:
		// mode 0
		bitwise::ClearBit(LCD_STATUS_MODE_FLAG_FIRST_BIT, m_LCDStatus);
		bitwise::ClearBit(LCD_STATUS_MODE_FLAG_SECOND_BIT, m_LCDStatus);
		m_Mode = Video_Mode::H_Blank;
		break;
	case GPU::Video_Mode::V_Blank:
		// mode 1
		bitwise::SetBit(LCD_STATUS_MODE_FLAG_FIRST_BIT, m_LCDStatus);
		bitwise::ClearBit(LCD_STATUS_MODE_FLAG_SECOND_BIT, m_LCDStatus);
		m_Mode = Video_Mode::V_Blank;
		break;
	case GPU::Video_Mode::Searching_OAM:
		// mode 2
		bitwise::ClearBit(LCD_STATUS_MODE_FLAG_FIRST_BIT, m_LCDStatus);
		bitwise::SetBit(LCD_STATUS_MODE_FLAG_SECOND_BIT, m_LCDStatus);
		m_Mode = Video_Mode::Searching_OAM;
		break;
	case GPU::Video_Mode::Transfer_Data_To_LCD:
		// mode 3
		bitwise::SetBit(LCD_STATUS_MODE_FLAG_FIRST_BIT, m_LCDStatus);
		bitwise::SetBit(LCD_STATUS_MODE_FLAG_SECOND_BIT, m_LCDStatus);
		m_Mode = Video_Mode::Transfer_Data_To_LCD;
		break;
	}
}

void GPU::ChangePalette(const Palette& i_Palette)
{
	switch (i_Palette)
	{
		case Palette::Pocket_Pallete:
			m_CurrPalette = Palette::Pocket_Pallete;
			break;
        case Palette::Original_Pallete:
			m_CurrPalette = Palette::Original_Pallete;
			break;
        case Palette::Autmn_Pallete:
			m_CurrPalette = Palette::Autmn_Pallete;
			break;
	}
	m_PalettePtr = Palettes[(int)i_Palette];
}

void GPU::ChangeToNextPalette()
{
	m_CurrPalette = Palette(((int)m_CurrPalette + 1) % NUM_OF_PALETTES);
	ChangePalette(m_CurrPalette);
}
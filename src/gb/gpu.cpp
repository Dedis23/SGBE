#include "gpu.h"

GPU::GPU(Gameboy& i_Gameboy) : m_Gameboy(i_Gameboy), m_IsLCDEnabled(true), m_Mode(Video_Mode::Searching_OAM), m_VideoCycles(0),
m_LCDControl(0), m_LCDStatus(0), m_ScrollY(0), m_ScrollX(0), m_LCDCYCoordinate(0), m_LYCompare(0), m_BGPaletteData(0),
m_ObjectPalette0(0), m_ObjectPalette1(0), m_WindowYPosition(0), m_WindowXPositionMinus7(0)
{
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
			//os << "m_BGPaletteData; " << std::dec << (int)m_BGPaletteData << std::endl;
			//os << "m_ObjectPalette0; " << std::dec << (int)m_ObjectPalette0 << std::endl;
			//os << "m_ObjectPalette1; " << std::dec << (int)m_ObjectPalette1 << std::endl;
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
			handleVBlankMode(i_Cycles);
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
	m_BGPaletteData = 0;
	m_ObjectPalette0 = 0;
	m_ObjectPalette1 = 0;
	m_WindowYPosition = 0;
	m_WindowXPositionMinus7 = 0;
	m_IsLCDEnabled = true;
	m_VideoCycles = 0;
	// reset all pixels to white 255 in RGB
	memset(m_FrameBuffer, 0xFF, sizeof(m_FrameBuffer));
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
	case GPU_BG_PALETTE_DATA_ADDR:
		{
			return m_BGPaletteData;
		}
		break;
	case GPU_OBJECT_PALETTE_0_DATA_ADDR:
		{
			return m_ObjectPalette0;
		}
		break;
	case GPU_OBJECT_PALETTE_1_DATA_ADDR:
		{
			return m_ObjectPalette1;
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
			m_Gameboy.GetMMU().DMATransfer(i_Value);
		}
		break;
	case GPU_BG_PALETTE_DATA_ADDR:
		{
			m_BGPaletteData = i_Value;
		}
		break;
	case GPU_OBJECT_PALETTE_0_DATA_ADDR:
		{
			m_ObjectPalette0 = i_Value;
		}
		break;
	case GPU_OBJECT_PALETTE_1_DATA_ADDR:
		{
			m_ObjectPalette1 = i_Value;
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
			m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::VBlank);

			// check for mode 1 interrupt bit
			if (bitwise::GetBit(LCDC_STATUS_MODE_1_V_BLANK_INTERRUPT_BIT, m_LCDStatus))
			{
				m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
			}

			// move to mode 1
			setMode(Video_Mode::V_Blank);
		}
		else // move again to OAM search (mode 2) to create the next scanline
		{
			// check for mode 2 interrupt bit
			if (bitwise::GetBit(LCDC_STATUS_MODE_2_OAM_INTERRUPT_BIT, m_LCDStatus))
			{
				m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
			}

			// move to mode 2
			setMode(Video_Mode::Searching_OAM);
		}
	}
}

/* mode 1 handler */
void GPU::handleVBlankMode(const uint32_t& i_Cycles)
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
		if (bitwise::GetBit(LCDC_STATUS_MODE_2_OAM_INTERRUPT_BIT, m_LCDStatus))
		{
			m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
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
		if (bitwise::GetBit(LCDC_STATUS_MODE_0_H_BLANK_INTERRUPT_BIT, m_LCDStatus))
		{
			m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
		}

		// move to mode 0 (H Blank)
		setMode(Video_Mode::H_Blank);
	}
}

/* draws a single scanline with background, window and sprite */
void GPU::drawCurrentScanline()
{
	// check BG/Window priority bit, if 0 then do not write BG and Window
	if (bitwise::GetBit(LCD_CONTROL_BG_WINDOW_DISPLAY_PRIORITY_BIT, m_LCDControl))
	{
		drawBackground();
		// check for window display bit
		if (bitwise::GetBit(LCD_CONTROL_WINDOW_DISPLAY_ENABLE_BIT, m_LCDControl))
		{
			drawWindow();
		}
	}
	// check for sprite bit
	if (bitwise::GetBit(LCD_CONTROL_SPRITE_DISPLAY_ENABLE_BIT, m_LCDControl))
	{
		drawSprites();
	}
}

void GPU::checkForLYAndLYCCoincidence()
{
	// compare the current Y scanline and LYC
	if (m_LCDCYCoordinate == m_LYCompare)
	{
		// raise coincidence bit
		bitwise::SetBit(LCDC_STATUS_LYC_LY_COINCIDENCE_FLAG_BIT, true, m_LCDStatus);
		if (bitwise::GetBit(LCDC_STATUS_LYC_EQUALS_LY_COINCIDENCE_INTERRUPT_BIT, m_LCDStatus))
		{
			m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
		}
	}
	else //  its > or <
	{
		// clear the coincidence bit
		bitwise::SetBit(LCDC_STATUS_LYC_LY_COINCIDENCE_FLAG_BIT, false, m_LCDStatus);
	}
}

void GPU::drawBackground()
{

}

/* the window is behind the sprites and above the background (unless specified otherwise)
   it is generally used for UI and its a fixed panel that will generaly not scroll */
void GPU::drawWindow()
{

}

void GPU::drawSprites()
{

}

void GPU::setMode(Video_Mode i_NewMode)
{
	// set bits 0 and 1 in status LCD register to the new mode
	switch (i_NewMode)
	{
	case GPU::Video_Mode::H_Blank:
		// mode 0
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_FIRST_BIT, false, m_LCDStatus);
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_SECOND_BIT, false, m_LCDStatus);
		m_Mode = Video_Mode::H_Blank;
		break;
	case GPU::Video_Mode::V_Blank:
		// mode 1
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_FIRST_BIT, true, m_LCDStatus);
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_SECOND_BIT, false, m_LCDStatus);
		m_Mode = Video_Mode::V_Blank;
		break;
	case GPU::Video_Mode::Searching_OAM:
		// mode 2
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_FIRST_BIT, false, m_LCDStatus);
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_SECOND_BIT, true, m_LCDStatus);
		m_Mode = Video_Mode::Searching_OAM;
		break;
	case GPU::Video_Mode::Transfer_Data_To_LCD:
		// mode 3
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_FIRST_BIT, true, m_LCDStatus);
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_SECOND_BIT, true, m_LCDStatus);
		m_Mode = Video_Mode::Transfer_Data_To_LCD;
		break;
	}
}
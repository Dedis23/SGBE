#include "gpu.h"

GPU::GPU(Gameboy& i_Gameboy) : m_Gameboy(i_Gameboy), m_Mode(Video_Mode::Searching_OAM), m_VideoCycles(0) {}

void GPU::Step(uint32_t& i_Cycles)
{
	if (isLCDEnabled())
	{
		m_VideoCycles += i_Cycles;
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
		default:
			break;
		}
	}
}

void GPU::Reset()
{
	m_Mode = Video_Mode::Searching_OAM;
	m_VideoCycles = 0;
}

bool GPU::isLCDEnabled()
{
	byte LCDC = m_Gameboy.GetMMU().Read(GPU_LCD_CONTROL_ADDR);
	return bitwise::GetBit(LCD_DISPLAY_ENABLE_BIT, LCDC);
}

void GPU::handleHBlankMode()
{

}

void GPU::handleVBlankMode()
{

}

void GPU::handleSearchSpritesAttributesMode()
{
	if (m_VideoCycles >= MIN_SEARCHING_OAM_MODE_CYCLES)
	{
		m_VideoCycles %= MIN_SEARCHING_OAM_MODE_CYCLES;
		// move to mode 3
		setMode(Video_Mode::Transfer_Data_To_LCD);
	}
}

void GPU::handleLCDTransferMode()
{

}

void GPU::setMode(Video_Mode i_NewMode)
{
	// set bits 0 and 1 in status LCD register to the new mode
	byte lcdcStatus = m_Gameboy.GetMMU().Read(GPU_LCDC_STATUS_ADDR);
	switch (i_NewMode)
	{
	case GPU::Video_Mode::H_Blank:
		// mode 0
		bitwise::SetBit(0, false, lcdcStatus);
		bitwise::SetBit(1, false, lcdcStatus);
		m_Mode = Video_Mode::H_Blank;
		break;
	case GPU::Video_Mode::V_Blank:
		// mode 1
		bitwise::SetBit(0, true, lcdcStatus);
		bitwise::SetBit(1, false, lcdcStatus);
		m_Mode = Video_Mode::V_Blank;
		break;
	case GPU::Video_Mode::Searching_OAM:
		// mode 2
		bitwise::SetBit(0, false, lcdcStatus);
		bitwise::SetBit(1, true, lcdcStatus);
		m_Mode = Video_Mode::Searching_OAM;
		break;
	case GPU::Video_Mode::Transfer_Data_To_LCD:
		// mode 3
		bitwise::SetBit(0, true, lcdcStatus);
		bitwise::SetBit(1, true, lcdcStatus);
		m_Mode = Video_Mode::Transfer_Data_To_LCD;
		break;
	}
	m_Gameboy.GetMMU().Write(GPU_LCDC_STATUS_ADDR, lcdcStatus);
}
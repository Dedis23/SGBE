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

void GPU::DisableLCD()
{
	// TODO
}

void GPU::handleHBlankMode()
{

}

void GPU::handleVBlankMode()
{

}

void GPU::handleSearchSpritesAttributesMode()
{

}

void GPU::handleLCDTransferMode()
{

}
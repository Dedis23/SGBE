#include "gpu.h"

GPU::GPU(Gameboy& i_Gameboy) : m_Gameboy(i_Gameboy) {}

void GPU::Step(uint32_t& o_Cycles)
{
	if (isLCDEnabled())
	{

	}
}

bool GPU::isLCDEnabled()
{
	byte LCDC = m_Gameboy.GetMMU().Read(GPU_LCD_CONTROL_ADDR);
	return bitwise::GetBit(LCD_DISPLAY_ENABLE_BIT, LCDC);
}

void GPU::DisableLCD()
{

}
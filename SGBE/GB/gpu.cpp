#include "gpu.h"

GPU::GPU(Gameboy& i_Gameboy) : m_Gameboy(i_Gameboy), m_Mode(Video_Mode::Searching_OAM), m_VideoCycles(0) {}


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
void GPU::Step(uint32_t& i_Cycles)
{
	// synchronization explained:
	// in order to synchronize between the CPU and GPU in the emulation,
	// each step we update the member m_VideoCycles by the cycles that the cpu did in the last step
	// only once the cpu have made enough (the minimum) cycles that correspond to the current mode, then we actualy do the mode operation
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
	return bitwise::GetBit(LCD_CONTROL_LCD_DISPLAY_ENABLE_BIT, LCDC);
}

/* mode 0 handler */
void GPU::handleHBlankMode()
{
	if (m_VideoCycles >= MIN_H_BLANK_MODE_CYCLES)
	{
		m_VideoCycles %= MIN_H_BLANK_MODE_CYCLES;

		// increment the y scanline
		byte currScanline = m_Gameboy.GetMMU().Read(GPU_LCDC_Y_COORDINATE_ADDR);
		currScanline++;
		m_Gameboy.GetMMU().Write(GPU_LCDC_Y_COORDINATE_ADDR, currScanline);

		// after a change in the scanline, check for LY and LYC coincidence interrupt
		checkForLYAndLYCCoincidence();

		// move either to mode 2 or mode 1, based on current scanline
		if (currScanline == V_BLANK_START_SCANLINE)
		{
			// check for mode 1 interrupt bit
			if (checkForLCDCInterrupt(LCDC_STATUS_MODE_1_V_BLANK_INTERRUPT_BIT))
			{
				m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
			}

			// move to mode 1
			setMode(Video_Mode::V_Blank);
		}
		else // move again to OAM search (mode 2) to create the next scanline
		{
			// check for mode 2 interrupt bit
			if (checkForLCDCInterrupt(LCDC_STATUS_MODE_2_OAM_INTERRUPT_BIT))
			{
				m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
			}

			// move to mode 2
			setMode(Video_Mode::Searching_OAM);
		}
	}
}

/* mode 1 handler */
void GPU::handleVBlankMode()
{
	if (m_VideoCycles >= MIN_V_BLANK_MODE_SINGLE_LINE_CYCLES)
	{
		m_VideoCycles %= MIN_V_BLANK_MODE_SINGLE_LINE_CYCLES;

		// increment the y corrdinate
		byte currScanline = m_Gameboy.GetMMU().Read(GPU_LCDC_Y_COORDINATE_ADDR);
		currScanline++;
		m_Gameboy.GetMMU().Write(GPU_LCDC_Y_COORDINATE_ADDR, currScanline);

		// after a change in the scanline, check for LY and LYC coincidence interrupt
		checkForLYAndLYCCoincidence();

		// check if its time to go back to line 0 and mode 2 (meaning a full frame have passed)
		if (currScanline < V_BLANK_END_SCANLINE)
		{
			// reset Y corrdinate to 0
			currScanline = 0;
			m_Gameboy.GetMMU().Write(GPU_LCDC_Y_COORDINATE_ADDR, currScanline);

			// after a change in the scanline, check for LY and LYC coincidence interrupt
			checkForLYAndLYCCoincidence();

			// check for mode 2 interrupt bit
			if (checkForLCDCInterrupt(LCDC_STATUS_MODE_2_OAM_INTERRUPT_BIT))
			{
				m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
			}

			// move to mode 2
			setMode(Video_Mode::Searching_OAM);
		}
	}
}

/* mode 2 handler */
void GPU::handleSearchSpritesAttributesMode()
{
	if (m_VideoCycles >= MIN_SEARCHING_OAM_MODE_CYCLES)
	{
		m_VideoCycles %= MIN_SEARCHING_OAM_MODE_CYCLES;

		// move to mode 3 (Transfer data to LCD)
		setMode(Video_Mode::Transfer_Data_To_LCD);
	}
}

/* mode 3 handler */
void GPU::handleLCDTransferMode()
{
	if (m_VideoCycles >= MIN_TRANSFER_DATA_TO_LCD_MODE_CYCLES)
	{
		m_VideoCycles %= MIN_TRANSFER_DATA_TO_LCD_MODE_CYCLES;

		// write a single scanline
		drawCurrentScanline();

		// check for mode 0 (H Blank) interrupt bit
		if (checkForLCDCInterrupt(LCDC_STATUS_MODE_0_H_BLANK_INTERRUPT_BIT))
		{
			m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
		}

		// move to mode 0 (H Blank)
		setMode(Video_Mode::H_Blank);
	}
}

void GPU::drawCurrentScanline()
{
	// TODO
}

bool GPU::checkForLCDCInterrupt(int i_InterruptBit)
{
	bool isInterruptBitRaised = false;
	byte lcdcStatus = m_Gameboy.GetMMU().Read(GPU_LCDC_STATUS_ADDR);
	isInterruptBitRaised = bitwise::GetBit(i_InterruptBit, lcdcStatus);
	return isInterruptBitRaised;
}

void GPU::checkForLYAndLYCCoincidence()
{
	byte currScanline = m_Gameboy.GetMMU().Read(GPU_LCDC_Y_COORDINATE_ADDR);
	byte LYCompare = m_Gameboy.GetMMU().Read(GPU_LY_COMPARE_ADDR);
	byte lcdcStatus = m_Gameboy.GetMMU().Read(GPU_LCDC_STATUS_ADDR);

	// compare the current Y scanline and LYC
	if (currScanline == LYCompare)
	{
		// raise coincidence bit
		bitwise::SetBit(LCDC_STATUS_LYC_LY_COINCIDENCE_FLAG_BIT, true, lcdcStatus);
		if (checkForLCDCInterrupt(LCDC_STATUS_LYC_EQUALS_LY_COINCIDENCE_INTERRUPT_BIT))
		{
			m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::LCD);
		}
	}
	else //  its > or <
	{
		// clear the coincidence bit
		bitwise::SetBit(LCDC_STATUS_LYC_LY_COINCIDENCE_FLAG_BIT, false, lcdcStatus);
	}
	m_Gameboy.GetMMU().Write(GPU_LCDC_STATUS_ADDR, lcdcStatus);
}

void GPU::setMode(Video_Mode i_NewMode)
{
	// set bits 0 and 1 in status LCD register to the new mode
	byte lcdcStatus = m_Gameboy.GetMMU().Read(GPU_LCDC_STATUS_ADDR);
	switch (i_NewMode)
	{
	case GPU::Video_Mode::H_Blank:
		// mode 0
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_FIRST_BIT, false, lcdcStatus);
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_SECOND_BIT, false, lcdcStatus);
		m_Mode = Video_Mode::H_Blank;
		break;
	case GPU::Video_Mode::V_Blank:
		// mode 1
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_FIRST_BIT, true, lcdcStatus);
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_SECOND_BIT, false, lcdcStatus);
		m_Mode = Video_Mode::V_Blank;
		break;
	case GPU::Video_Mode::Searching_OAM:
		// mode 2
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_FIRST_BIT, false, lcdcStatus);
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_SECOND_BIT, true, lcdcStatus);
		m_Mode = Video_Mode::Searching_OAM;
		break;
	case GPU::Video_Mode::Transfer_Data_To_LCD:
		// mode 3
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_FIRST_BIT, true, lcdcStatus);
		bitwise::SetBit(LCDC_STATUS_MODE_FLAG_SECOND_BIT, true, lcdcStatus);
		m_Mode = Video_Mode::Transfer_Data_To_LCD;
		break;
	}
	m_Gameboy.GetMMU().Write(GPU_LCDC_STATUS_ADDR, lcdcStatus);
}
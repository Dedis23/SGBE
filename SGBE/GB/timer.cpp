#include "timer.h"

const uint32_t Timer::CyclesArr[] = { 1024, 16, 64, 256 };

// Note - the divider freq is 16384 and cannot be set otherwise
Timer::Timer(Gameboy& i_Gameboy) : m_Gameboy(i_Gameboy), m_IsEnabled(true), 
m_RemainingCyclesToTickTheCounter(CyclesArr[(int)TimerFrequencies::_4096Hz]),
m_RemainingCyclesToTickTheDivider(CyclesArr[(int)TimerFrequencies::_16384Hz]),
m_CurrentFrequency(TimerFrequencies::_4096Hz),
m_DividerCounter(0), m_TimerCounter(0), m_TimerModulo(0), m_TimerControl(0b000000100) {}

void Timer::Step(const uint32_t& i_Cycles)
{
	// tick the timer counter only if it is enabled
	if (m_IsEnabled)
	{
		// decrease cycles
		m_RemainingCyclesToTickTheCounter -= i_Cycles;
		if (m_RemainingCyclesToTickTheCounter <= 0)
		{
			// tick the counter
			m_TimerCounter++;
			if (m_TimerCounter == 0)
			{
				// Timer overflow occured
				// set the timer to the modulo
				m_TimerCounter = m_TimerModulo;
				
				// trigger the timer interrupt
				m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::Timer);
			}
		}
	}

	// tick the divider regardless of the timer state
	// it will overflow eventually and will start counting again from 0 (0-255)
	// here no timer interrupts will be invoked
	m_RemainingCyclesToTickTheDivider -= i_Cycles;
	if (m_RemainingCyclesToTickTheDivider <= 0)
	{
		m_RemainingCyclesToTickTheDivider = CyclesArr[(int)TimerFrequencies::_16384Hz];
		m_DividerCounter++;
	}
}

void Timer::Reset()
{
	m_IsEnabled = true;
	m_RemainingCyclesToTickTheCounter = CyclesArr[(int)TimerFrequencies::_4096Hz];
	m_RemainingCyclesToTickTheDivider = CyclesArr[(int)TimerFrequencies::_16384Hz];
	m_CurrentFrequency = TimerFrequencies::_4096Hz;
	// reset timer memory registers in memory
	m_DividerCounter = 0;
	m_TimerCounter = 0;
	m_TimerModulo = 0;
	m_TimerControl = 0b000000100; // the default value for the timer controler, is enabled and 4096Hz mode
}

byte Timer::GetRegister(const word& i_Address) const
{
	switch (i_Address)
	{
	case TIMER_DIVIDER_ADDR:
		{
			return m_DividerCounter;
		}
		break;
	case TIMER_COUNTER_ADDR:
		{
			return m_TimerCounter;
		}
		break;
	case TIMER_MODULO_ADDR:
		{
			return m_TimerModulo;
		}
		break;
	case TIMER_CONTROL_ADDR:
		{
			return m_TimerControl;
		}
		break;
	}

	LOG_ERROR(true, return 0, "Attempting to read from unmapped memory address: 0x" << i_Address);
}

void Timer::SetRegister(const word& i_Address, byte i_Value)
{
	switch (i_Address)
	{
	case TIMER_DIVIDER_ADDR:
		{
			m_DividerCounter = 0;
		}
		break;
	case TIMER_COUNTER_ADDR:
		{
			m_TimerCounter = i_Value;
		}
		break;
	case TIMER_MODULO_ADDR:
		{
			m_TimerModulo = i_Value;
		}
		break;
	case TIMER_CONTROL_ADDR:
		{
			m_IsEnabled = bitwise::GetBit(TIMER_CONTROL_ENABLE_BIT, i_Value) ? true : false; // read bit 2 for enable/disable
			Timer::TimerFrequencies freq = (Timer::TimerFrequencies)(i_Value & 0x3); // get bits 0 and 1 for freq type
			setFrequency(freq);
		}
		break;
	}
}

void Timer::setFrequency(TimerFrequencies i_Frequency)
{
	if (i_Frequency != m_CurrentFrequency) // set only if a change was detected
	{
		switch (i_Frequency)
		{
		case Timer::TimerFrequencies::_4096Hz:
			m_RemainingCyclesToTickTheCounter = CyclesArr[(int)Timer::TimerFrequencies::_4096Hz];
			break;
		case Timer::TimerFrequencies::_262144Hz:
			m_RemainingCyclesToTickTheCounter = CyclesArr[(int)Timer::TimerFrequencies::_262144Hz];
			break;
		case Timer::TimerFrequencies::_65536Hz:
			m_RemainingCyclesToTickTheCounter = CyclesArr[(int)Timer::TimerFrequencies::_65536Hz];
			break;
		case Timer::TimerFrequencies::_16384Hz:
			m_RemainingCyclesToTickTheCounter = CyclesArr[(int)Timer::TimerFrequencies::_16384Hz];
			break;
		}
	}
}
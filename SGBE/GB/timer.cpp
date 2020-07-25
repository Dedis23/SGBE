#include "timer.h"

const uint32_t Timer::CyclesArr[] = { CPU_CLOCK_SPEED / 4096, CPU_CLOCK_SPEED / 262144, CPU_CLOCK_SPEED / 65536, CPU_CLOCK_SPEED / 16384 };

// Note - the divider freq is 16384 and cannot be set otherwise
Timer::Timer(Gameboy& i_Gameboy) : m_Gameboy(i_Gameboy), m_IsEnabled(true), 
m_RemainingCyclesToTickTheCounter(CyclesArr[(int)TimerFrequencies::_4096Hz]),
m_RemainingCyclesToTickTheDivider(CyclesArr[(int)TimerFrequencies::_16384Hz]),
m_CurrentFrequency(TimerFrequencies::_4096Hz)
{
	// the default of the timer controler is enabled and set to 4096
	m_Gameboy.GetMMU().Write(TIMER_CONTROL_ADDR, 0b000000100);
}

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
			byte timerCounter = m_Gameboy.GetMMU().Read(TIMER_COUNTER_ADDR);
			timerCounter++;
			if (timerCounter == 0)
			{
				// Timer overflow occured
				// set the timer to the modulo
				timerCounter = m_Gameboy.GetMMU().Read(TIMER_MODULO_ADDR);
				
				// trigger the timer interrupt
				m_Gameboy.GetCPU().RequestInterrupt(CPU::InterruptType::Timer);
			}
			m_Gameboy.GetMMU().Write(TIMER_COUNTER_ADDR, timerCounter);
		}
	}

	// tick the divider regardless of the timer state
	m_RemainingCyclesToTickTheDivider -= i_Cycles;
	if (m_RemainingCyclesToTickTheDivider <= 0)
	{
		m_RemainingCyclesToTickTheDivider = CyclesArr[(int)TimerFrequencies::_16384Hz];
		// it will overflow eventually and will start counting again from 0 (0-255)
		// here no timer interrupts will be invoked
		byte dividerCounter = m_Gameboy.GetMMU().Read(TIMER_DIVIDER_ADDR);
		dividerCounter++;
		m_Gameboy.GetMMU().Write(TIMER_DIVIDER_ADDR, dividerCounter);
	}
}

void Timer::SetTimerControl(byte i_NewTimerControl)
{
	bitwise::GetBit(2, i_NewTimerControl) ? start() : stop(); // read bit 2 for enable/disable
	Timer::TimerFrequencies freq = (Timer::TimerFrequencies)(i_NewTimerControl & 0x3); // get bits 0 and 1 for freq type
	setFrequency(freq);
}

void Timer::setFrequency(TimerFrequencies i_Frequency)
{
	if (i_Frequency != m_CurrentFrequency) // reset only if a change was detected
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

void Timer::start()
{
	m_IsEnabled = true;
}

void Timer::stop()
{
	m_IsEnabled = false;
}
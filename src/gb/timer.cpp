#include "timer.h"

const uint32_t Timer::CyclesArr[] = { 1024, 16, 64, 256 };

// Note - the divider freq is 16384Hz and cannot be set otherwise
Timer::Timer(GBInternals& i_GBInternals) :
m_IsEnabled(true),
m_CurrentFrequency(TimerFrequencies::_4096Hz),
m_TimerCounterCycles(0),
m_DividerCycles(0),
m_DividerCyclesUntillTick(CyclesArr[(int)TimerFrequencies::_16384Hz]),
m_DividerCounter(0), 
m_TimerCounter(0),
m_TimerModulo(0), 
m_TimerControl(4),
m_GBInternals(i_GBInternals) {}

void Timer::Step(const uint32_t& i_Cycles)
{
	// tick the timer counter only if it is enabled
	if (m_IsEnabled)
	{
		// increment cycles
		m_TimerCounterCycles += i_Cycles;

		// get current freq in cycles
		uint32_t currFreqCycles = CyclesArr[(int)m_CurrentFrequency];

		while (m_TimerCounterCycles >= currFreqCycles)
		{
			m_TimerCounterCycles -= currFreqCycles;

			// tick the counter
			m_TimerCounter++;
			if (m_TimerCounter == 0)
			{
				// Timer overflow occured
				// set the timer to the modulo
				m_TimerCounter = m_TimerModulo;
				
				// trigger the timer interrupt
				m_GBInternals.GetCPU().RequestInterrupt(CPU::InterruptType::Timer);
			}
		}
	}

	// tick the divider regardless of the timer state
	// it will overflow eventually and will start counting again from 0 (0-255)
	// here no timer interrupts will be invoked
	// it always operates in 4096Hz
	m_DividerCycles += i_Cycles;
	while (m_DividerCycles >= m_DividerCyclesUntillTick)
	{
		m_DividerCycles -= m_DividerCyclesUntillTick;
		m_DividerCounter++;
	}
}

void Timer::Reset()
{
	m_IsEnabled = true;
	m_DividerCycles = 0;
	m_TimerCounterCycles = 0;
	m_CurrentFrequency = TimerFrequencies::_4096Hz;
	// reset timer memory registers in memory
	m_DividerCounter = 0;
	m_TimerCounter = 0;
	m_TimerModulo = 0;
	m_TimerControl = 4; // the default value for the timer controler, is enabled and in 4096Hz mode
}

byte Timer::GetRegister(word i_Address) const
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

	LOG_ERROR(true, return 0, "Attempting to read from unmapped memory address: 0x" << std::hex << i_Address);
}

void Timer::SetRegister(word i_Address, byte i_Value)
{
	switch (i_Address)
	{
	case TIMER_DIVIDER_ADDR:
		{
			m_DividerCounter = 0;
			return;
		}
		break;
	case TIMER_COUNTER_ADDR:
		{
			m_TimerCounter = i_Value;
			return;
		}
		break;
	case TIMER_MODULO_ADDR:
		{
			m_TimerModulo = i_Value;
			return;
		}
		break;
	case TIMER_CONTROL_ADDR:
		{
			m_IsEnabled = bitwise::IsBitSet(TIMER_CONTROL_ENABLE_BIT, i_Value) ? true : false; // read bit 2 for enable/disable
			m_CurrentFrequency = (Timer::TimerFrequencies)(i_Value & 0x3); // get bits 0 and 1 for freq type
			return;
		}
		break;
	}

	LOG_ERROR(true, return, "Attempting to write to an unmapped memory address: 0x" << std::hex << i_Address);
}
#include "timer.h"

const uint32_t Timer::TimerFrequenciesArr[] = { 4096, 262144, 65536, 16384 };

// Note - the divider freq is 16384 and cannot be set otherwise
Timer::Timer() : m_IsEnabled(true), m_RemainingCyclesToTickTheCounter(0),
m_RemainingCyclesToTickTheDivider(TimerFrequenciesArr[(int)TimerFrequencies::_16384Hz]),
m_TimerCounter(0), m_DividerCounter(0), m_TimerModulo(0), m_TimerControl(0),
m_CurrentFrequency(TimerFrequencies::_4096Hz) {}

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
				//TODO
			}
		}
	}

	// tick the divider regardless of the timer state
	m_RemainingCyclesToTickTheDivider -= i_Cycles;
	if (m_RemainingCyclesToTickTheDivider <= 0)
	{
		// it will overflow eventually and will start counting again from 0 (0-255)
		// here no timer interrupts will be invoked
		m_DividerCounter++; 
	}
}

void Timer::SetTimerControl(byte i_NewTimerControl)
{
	m_TimerControl = i_NewTimerControl;
}

byte Timer::GetTimerControl() const
{
	return m_TimerControl;
}

void Timer::SetTimerCounter(byte i_NewTimerCounter)
{
	m_TimerCounter = i_NewTimerCounter;

	bitwise::GetBit(2, i_NewTimerCounter) ? start() : stop();
	Timer::TimerFrequencies freq = (Timer::TimerFrequencies)(i_NewTimerCounter & 0x2);
	setFrequency(freq);
}

byte Timer::GetTimerCounter() const
{
	return m_TimerCounter;
}

void Timer::ResetDividerTimer()
{
	m_DividerCounter = 0;
}

byte Timer::GetDividerCounter() const
{
	return m_DividerCounter;
}

void Timer::SetTimerModulo(byte i_NewTimerModulo)
{
	m_TimerModulo = i_NewTimerModulo;
}

byte Timer::GetTimerModulo() const
{
	return m_TimerModulo;
}

void Timer::setFrequency(TimerFrequencies i_Frequency)
{
	if (i_Frequency != m_CurrentFrequency) // reset only if a change was detected
	{
		switch (i_Frequency)
		{
		case Timer::TimerFrequencies::_4096Hz:
			m_RemainingCyclesToTickTheCounter = TimerFrequenciesArr[(int)Timer::TimerFrequencies::_4096Hz];
			break;
		case Timer::TimerFrequencies::_262144Hz:
			m_RemainingCyclesToTickTheCounter = TimerFrequenciesArr[(int)Timer::TimerFrequencies::_262144Hz];
			break;
		case Timer::TimerFrequencies::_65536Hz:
			m_RemainingCyclesToTickTheCounter = TimerFrequenciesArr[(int)Timer::TimerFrequencies::_65536Hz];
			break;
		case Timer::TimerFrequencies::_16384Hz:
			m_RemainingCyclesToTickTheCounter = TimerFrequenciesArr[(int)Timer::TimerFrequencies::_16384Hz];
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
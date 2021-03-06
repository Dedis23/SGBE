/*******************************************************
 *			Created by: Dedi Sidi, 2020                *
 *												       *
 *			Timer - internal timer of the gameboy	   *
 *******************************************************/

 /* Most of the information were take from the ultimate gameboy talk and codeslinger blog *
  * https://www.youtube.com/watch?v=HyzD8pNlpwI		                                      *
  * https://gbdev.io/pandocs/#timer-and-divider-registers                                 *
  * http://www.codeslinger.co.uk/pages/projects/gameboy/timers.html                       */

#pragma once
#include "utility.h"
#include "cpu.h"

/* Timer registers addresses in memory */
const word TIMER_DIVIDER_ADDR = 0xFF04; // increment at a steady rate, regardless of if timer is enabled and mode
const word TIMER_COUNTER_ADDR = 0xFF05; // the counter itself
const word TIMER_MODULO_ADDR = 0xFF06;  // when the the counter overflows, it will get this value
const word TIMER_CONTROL_ADDR = 0xFF07; // holds the freq mode and  whether if tis enabled

/* Timer Control bits */
#define TIMER_CONTROL_ENABLE_BIT             2 // 0 off, 1 on
#define TIMER_CONTROL_MODE_FLAG_SECOND_BIT   1 // 00: 4096Hz 01: 262144Hz 
#define TIMER_CONTROL_MODE_FLAG_FIRST_BIT    0 // 10: 65536Hz 11: 16384Hz

class GBInternals;

class Timer
{
public:
	enum class TimerFrequencies
	{
        _4096Hz = 0,
        _262144Hz = 1,
        _65536Hz = 2,
        _16384Hz = 3,
	};

    static const uint32_t CyclesArr[];

public:
    Timer(GBInternals& i_GBInternals);
	virtual ~Timer() = default;
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void Step(const uint32_t& i_Cycles);
    void Reset();
    byte GetRegister(word i_Address) const;
    void SetRegister(word i_Address, byte i_Value);

private:
    bool m_IsEnabled;
    TimerFrequencies m_CurrentFrequency;
    int32_t m_TimerCounterCycles;
    int32_t m_DividerCycles;
    const uint32_t m_DividerCyclesUntillTick;
    byte m_DividerCounter;
    byte m_TimerCounter;
    byte m_TimerModulo;
    byte m_TimerControl;

	/* gameboy components ref */
	GBInternals& m_GBInternals;
};
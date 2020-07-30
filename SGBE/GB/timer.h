/*******************************************************
 *			Created by: Dedi Sidi, 2020                *
 *												       *
 *			Timer - internal timer of the gameboy	   *
 *******************************************************/

 /* Most of the information were take from the ultimate gameboy talk and codeslinger blog *
  * https://www.youtube.com/watch?v=HyzD8pNlpwI		                                      *
  * https://gbdev.gg8.se/wiki/articles/Timer_and_Divider_Registers                        *
  * http://www.codeslinger.co.uk/pages/projects/gameboy/timers.html                       */

#pragma once
#include "utility.h"
#include "cpu.h"

/* Timer registers addresses in memory */
const word TIMER_DIVIDER_ADDR = 0xFF04; // R/W
const word TIMER_COUNTER_ADDR = 0xFF05; // R/W
const word TIMER_MODULO_ADDR = 0xFF06;  // R/W
const word TIMER_CONTROL_ADDR = 0xFF07; // R/W

class Gameboy;

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
    Timer(Gameboy& i_Gameboy);
	virtual ~Timer() = default;
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void Step(const uint32_t& i_Cycles);
    void Reset();
    void SetTimerControl(byte i_NewTimerControl);

private:
    void setFrequency(TimerFrequencies i_Frequency);
    void start();
    void stop();

private:
    bool m_IsEnabled;
    TimerFrequencies m_CurrentFrequency;
    int32_t m_RemainingCyclesToTickTheCounter;
    int32_t m_RemainingCyclesToTickTheDivider;

    /* gameboy ref */
    Gameboy& m_Gameboy;
};
/*******************************************************
 *			Created by: Dedi Sidi, 2020                *
 *												       *
 *			Timer - internal timer of the gameboy	   *
 *******************************************************/

 /* Most of the information were take from the ultimate gameboy talk and codeslinger blog *
  * https://www.youtube.com/watch?v=HyzD8pNlpwI		                                      *
  * http://www.codeslinger.co.uk/pages/projects/gameboy/timers.html                       */

#include "utility.h"
#include "cpu.h"

#ifndef __TIMER_H
#define __TIMER_H

const word DIVIDER_REGISTER_ADDR = 0xFF04;
const word TIMER_COUNTER_ADDR = 0xFF05;
const word TIMER_MODULO_ADDR = 0xFF06;
const word TIMER_CONTROL_ADDR = 0xFF07;

class Timer
{
public:
    Timer() = default;
	virtual ~Timer() = default;
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void Step(uint32_t& o_Cycles);

private:
    uint32_t m_CyclesToTick;
};

#endif
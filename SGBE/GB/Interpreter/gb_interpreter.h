/********************************************************************************
 *			Copyright (C) 2020 Dedi Sidi										*
 *																				*
 *			The interpreter is the main emulation module						*
 *			It holds and operates the various components of the gameboy			*
 ********************************************************************************/

#ifndef __GB_INTERPRETER_H
#define __GB_INTERPRETER_H

#include "../CPU/gb_cpu.h"

class GBInterpreter
{
public:
	GBInterpreter() = default;
	virtual ~GBInterpreter() = default;
	GBInterpreter(const GBInterpreter&) = delete;
	GBInterpreter& operator=(const GBInterpreter&) = delete;

	void Run();

private:
	GBCPU m_CPU;
};

#endif
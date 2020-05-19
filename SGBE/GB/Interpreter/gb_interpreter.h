/********************************************************************************
 *			Copyright (C) 2020 Dedi Sidi										*
 *																				*
 *			The interpreter is the main emulation module						*
 *			It holds and operates the various components of the gameboy			*
 ********************************************************************************/

#ifndef __GB_INTERPRETER_H
#define __GB_INTERPRETER_H

#include <string>
#include <vector>
#include "../CPU/gb_cpu.h"
#include "../../Generic/logger.h"
#include "../gb_cartridge.h"

class GBInterpreter
{
public:
	GBInterpreter(const std::string& i_RomFileName);
	virtual ~GBInterpreter() = default;
	GBInterpreter(const GBInterpreter&) = delete;
	GBInterpreter& operator=(const GBInterpreter&) = delete;

	void Run();

private:
	GBCPU m_CPU;
	GBCartridge m_Cartridge;
};

#endif
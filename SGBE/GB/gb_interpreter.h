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
#include "CPU/gb_cpu.h"
#include "gb_utility.h"
#include "Cartridge/gb_cartridge.h"
#include "Cartridge//þþgb_cartridge_metadata.h"
#include "þþgb_mmu.h"

class GBInterpreter
{
public:
	GBInterpreter();
	virtual ~GBInterpreter();
	GBInterpreter(const GBInterpreter&) = delete;
	GBInterpreter& operator=(const GBInterpreter&) = delete;

	bool Initialize(const std::string& i_RomFileName);
	void Run();

private:
	bool loadROM(const string& i_RomFilePath);
	bool initializeCartridge();

private:
	vector<byte> m_ROMData;
	GBCPU m_CPU;
	GBMMU m_MMU;
	GBCartridge* m_Cartridge;
};

#endif
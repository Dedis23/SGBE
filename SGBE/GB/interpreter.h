/********************************************************************************
 *			Copyright (C) 2020 Dedi Sidi										*
 *																				*
 *			The interpreter is the main emulation module						*
 *			It holds and operates the various components of the gameboy			*
 ********************************************************************************/

#ifndef __INTERPRETER_H
#define __INTERPRETER_H

#include <string>
#include <vector>
#include "cpu.h"
#include "utility.h"
#include "cartridge.h"
#include "cartridge_header.h"
#include "mmu.h"

class Interpreter
{
public:
	Interpreter();
	virtual ~Interpreter();
	Interpreter(const Interpreter&) = delete;
	Interpreter& operator=(const Interpreter&) = delete;

	bool Initialize(const std::string& i_RomFileName);
	bool IsCartridgeLoadedSuccessfully();
	void Run();

private:
	bool loadROM(const string& i_RomFilePath);
	bool initializeCartridge();

private:
	vector<byte> m_ROMData;
	CPU m_CPU;
	MMU m_MMU;
	Cartridge* m_Cartridge;
};

#endif
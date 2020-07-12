/********************************************************************************
 *			Created by: Dedi Sidi, 2020											*
 *																				*
 *			The interpreter is the main emulation module						*
 *			It holds and operates the various components of the gameboy			*
 ********************************************************************************/

#pragma once
#include <string>
#include <vector>
#include "cpu.h"
#include "cartridge.h"
#include "mmu.h"
#include "ppu.h"

class Interpreter
{
public:
	typedef void(*DrawFunction)(byte i_R, byte i_G, byte i_B, uint32_t i_WidthPosition, uint32_t i_HeightPosition);

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
	CPU* m_CPU;
	MMU* m_MMU;
	PPU* m_PPU;
	CartridgeHeader* m_CartridgeHeader;
	Cartridge* m_Cartridge;
	DrawFunction m_DrawFunction;
};
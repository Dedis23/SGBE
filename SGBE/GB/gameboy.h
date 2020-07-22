/********************************************************************************
 *			Created by: Dedi Sidi, 2020											*
 *																				*
 *			The Gameboy class is the main emulation module						*
 *			It holds and operates the various components of the gameboy			*
 ********************************************************************************/

#pragma once
#include <string>
#include <vector>
#include "cpu.h"
#include "cartridge.h"
#include "mmu.h"
#include "gpu.h"
#include "timer.h"

class CPU;
class MMU;
class Timer;

class Gameboy
{
public:
	typedef void(*DrawFunction)(byte i_R, byte i_G, byte i_B, uint32_t i_WidthPosition, uint32_t i_HeightPosition);

public:
	Gameboy(vector<byte>& i_ROMData);
	virtual ~Gameboy();
	Gameboy(const Gameboy&) = delete;
	Gameboy& operator=(const Gameboy&) = delete;

	bool Initialize();
	bool IsCartridgeLoadedSuccessfully();
	void Run();
	CPU& GetCPU();
	Timer& GetTimer();

private:
	bool initializeCartridge();

private:
	// internal parts
	CPU* m_CPU;
	MMU* m_MMU;
	GPU* m_GPU;
	Timer* m_Timer;
	// user input cartridge data
	vector<byte>& m_ROMData;
	CartridgeHeader* m_CartridgeHeader;
	Cartridge* m_Cartridge;
	// host specific methods
	DrawFunction m_DrawFunction;
};
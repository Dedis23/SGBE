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
#include "joypad.h"
#include <functional>
#include "utility.h"

class CPU;
class MMU;
class GPU;
class Timer;
class Joypad;
struct Pixel;

class Gameboy
{
public:
	Gameboy(vector<byte>& i_ROMData, function<void(const Pixel* i_FrameBuffer)> i_RenderFuncPtr);
	virtual ~Gameboy();
	Gameboy(const Gameboy&) = delete;
	Gameboy& operator=(const Gameboy&) = delete;

	bool Initialize();
	bool IsCartridgeLoadedSuccessfully();
	void Step();
	CPU& GetCPU();
	MMU& GetMMU();
	GPU& GetGPU();
	Timer& GetTimer();
	Joypad& GetJoypad();

private:
	bool initializeCartridge();

private:
	// internal parts
	CPU* m_CPU;
	MMU* m_MMU;
	GPU* m_GPU;
	Timer* m_Timer;
	Joypad* m_Joypad;
	// user input cartridge data
	vector<byte>& m_ROMData;
	CartridgeHeader* m_CartridgeHeader;
	Cartridge* m_Cartridge;
	// host specific methods
	function<void(const Pixel* i_FrameBuffer)> m_RenderScreen;
};
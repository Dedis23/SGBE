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
#include <functional>

class CPU;
class MMU;
class GPU;
class Timer;
struct Pixel;

//typedef void(*RenderFunction)(Pixel i_FrameBuffer[]);
std::function<void(Pixel i_FrameBuffer[])> t;

class Gameboy
{
public:
	Gameboy(vector<byte>& i_ROMData, RenderFunction i_RenderFunction);
	virtual ~Gameboy();
	Gameboy(const Gameboy&) = delete;
	Gameboy& operator=(const Gameboy&) = delete;

	bool Initialize();
	bool IsCartridgeLoadedSuccessfully();
	void Step();
	CPU& GetCPU();
	MMU& GetMMU();
	Timer& GetTimer();
	GPU& GetGPU();

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
	RenderFunction m_RenderFunction;
};
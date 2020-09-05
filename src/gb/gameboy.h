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
enum class GBButtons;
enum class Palette;
struct Pixel;

/* this class holds a reference to all the internal parts of the gameboy 
 * I added this so the internal parts will be able to have access to any other internal part without getting it directly from the Gameboy class
 * this way the host application will get a clean Gameboy API which not hold any method that will give access to its interal parts */
class GBInternals
{
public:
	GBInternals() : m_MMU(nullptr), m_CPU(nullptr), m_GPU(nullptr), m_Timer(nullptr), m_Joypad(nullptr) {}
	virtual ~GBInternals() = default;
	GBInternals(const GBInternals&) = delete;
	GBInternals& operator=(const GBInternals&) = delete;

	void AttachMMU(MMU* i_MMU) { m_MMU = i_MMU; }
	void AttachCPU(CPU* i_CPU) { m_CPU = i_CPU; }
	void AttachGPU(GPU* i_GPU) { m_GPU = i_GPU; }
	void AttachTimer(Timer* i_Timer) { m_Timer = i_Timer; }
	void AttachJoypad(Joypad* i_Joypad) { m_Joypad = i_Joypad; }
	
	MMU& GetMMU() { return *m_MMU; }
	CPU& GetCPU() { return *m_CPU; }
	GPU& GetGPU() { return *m_GPU; }
	Timer& GetTimer() { return *m_Timer; }
	Joypad& GetJoypad() { return *m_Joypad; }

private:
	MMU* m_MMU;
	CPU* m_CPU;
	GPU* m_GPU;
	Timer* m_Timer;
	Joypad* m_Joypad;
};

class Gameboy
{
public:
	Gameboy(vector<byte>& i_ROMData, function<void(const Pixel* i_FrameBuffer)> i_RenderFuncPtr);
	virtual ~Gameboy();
	Gameboy(const Gameboy&) = delete;
	Gameboy& operator=(const Gameboy&) = delete;

	/* gameboy API */
	bool Initialize();
	bool IsCartridgeLoadedSuccessfully();
	void Step(); // step a single frame
	void KeyPressed(const GBButtons& i_PressedButton) const;
	void KeyReleased(const GBButtons& i_ReleasedButton) const;
	void ChangePalette(const Palette& i_Palette);
	void ChangeToNextPalette();

private:
	bool initializeCartridge();

private:
	// internal parts
	MMU* m_MMU;
	CPU* m_CPU;
	GPU* m_GPU;
	Timer* m_Timer;
	Joypad* m_Joypad;
	GBInternals m_GBInternals;
	// user input cartridge data
	vector<byte>& m_ROMData;
	CartridgeHeader* m_CartridgeHeader;
	Cartridge* m_Cartridge;
	// host specific methods
	function<void(const Pixel* i_FrameBuffer)> m_RenderScreen;
};
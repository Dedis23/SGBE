/************************************************************
 *			Created by: Dedi Sidi, 2020						*
 *															*
 *			The gameboy memory management unit				*
 ************************************************************/

/* 
 The memory map of the gameboy:
 
 0x000-0x00FF: 256 bytes of the bootstrap - after reading that, the gameboy can access these addresses again from the cartridge
 
 0x0000-0x3FFF: 16K ROM bank (first rom bank of the cartridge)
   ---> 0x0100-0x014F: Cartridge header
 
 0x4000-0x7FFF: Switchable 16K ROM banks (the cartridge can switch rom banks so that the cpu can access more memory. (up to 256 banks) minimum cartridge 32KB max 4MB
 
 0x8000-0x9FFF: 8K Video RAM (internal video memory)
 
 0xA000-0xBFFF: 8K External RAM banks (the cartridge can have additional RAM banks that the cpu can access, up to 4 banks) 
 
 0xC000-0xDFFF: 8K Internal RAM
 
 0xE000-0xFDFF: Shadow RAM - exact copy of the internal RAM in: C000-DDFF - (7680 bytes excluding the higher 512 bytes)
 
 0xFE00-0xFE9F: OAM - "Object Attribute Memory" - stores information about the sprites
 
 0xFEA0-0xFEFF: Unusable memory
 
 0xFF00-0xFF7F: MappedIO - holds controls values which are releated to I/O components
 
 0xFF80-0xFFFF: Zero Page - high speed RAM of 128 bytes, includes the Interrupt Enable Register
*/

/*
    info mostly taken from:
    http://bgb.bircd.org/pandocs.htm#gameboytechnicaldata
    http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-Memory
	https://realboyemulator.wordpress.com/2013/01/03/a-look-at-the-game-boy-bootstrap-let-the-fun-begin/
*/

#pragma once
#include <string>
#include <vector>
#include "gameboy.h"
#include "cartridge/cartridge.h"
#include "utility.h"
#include "timer.h"

/* special locations addresses in memory */
const word BOOTSTRAP_DONE_ADDR = 0xFF50;

/* serial data transfer addresses in memory */
const word SERIAL_TRANSFER_DATA_ADDR = 0xFF01;
const word SERIAL_TRANSFER_CONTROL_ADDR = 0xFF02;

/* size of memory components */
const word VRAM_SIZE = (0x9FFF - 0x8000 + 1);
const word RAM_SIZE = (0xFDFF - 0xC000 + 1); // including the shadow ram
const word OAM_SIZE = (0xFEFF - 0xFE00 + 1); // including the unusable section
const word MAPPED_IO_SIZE = (0xFF7F - 0xFF00 + 1);
const word ZERO_PAGE_SIZE = (0xFFFF - 0xFF80 + 1);

class GBInternals;

class MMU
{
public:
    MMU(GBInternals& i_GBInternals, Cartridge& i_Cartridge);
	virtual ~MMU() = default;
    MMU(const MMU&) = delete;
    MMU& operator=(const MMU&) = delete;
	
	byte Read(word i_Address) const;
	void Write(word i_Address, byte i_Value);
	void DMATransfer(byte i_SourceAdress);

private:
	byte readMappedIO(word i_Address) const;
	void writeMappedIO(word i_Address, byte i_Value);
	bool isBootstrapDone() const;

private:
	/* memory modules */
    Cartridge& m_Cartridge;
	vector<byte> m_VRAM = vector<byte>(VRAM_SIZE, 0);
	vector<byte> m_RAM = vector<byte>(RAM_SIZE, 0); // including the shadow ram
	vector<byte> m_OAM = vector<byte>(OAM_SIZE, 0); // including the unusable section
	vector<byte> m_MappedIO = vector<byte>(MAPPED_IO_SIZE, 0);
	vector<byte> m_ZeroPageRAM = vector<byte>(ZERO_PAGE_SIZE, 0);
	static const vector<byte> s_Bootstrap;

	/* gameboy components ref */
	GBInternals& m_GBInternals;
};
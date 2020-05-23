/************************************************************
 *			Copyright (C) 2020 Dedi Sidi					*
 *															*
 *			The gameboy memory management unit				*
 ************************************************************/

/* 
	The memory map of the gameboy:

	0x000-0x00FF: 256 bytes of the bootstrap - after reading that, the gameboy can access these addresses again from the cartridge

	0x0000-0x3FFF: 16K ROM bank (first rom bank of the cartridge)
	  ---> 0x0100-0x014F: Cartridge header

	0x4000-0x7FFF: Switchable 16K ROM banks (the cartridge can switch rom banks so that the cpu can access more memory. (up to 256 banks) minimum cartridge 32KB max 4MB

	0x8000-0x9FFF: Video RAM (internal video memory)

	0xA000-0xBFFF: 8K External RAM banks (the cartridge can have additional RAM banks that the cpu can access, up to 4 banks) 

	0xC000-0xDFFF: 8K Internal RAM

	0xE000-0xFDFF: Shadow RAM - exact copy of the internal RAM - 7680 bytes excluding the higher 512 bytes. (this is due to a wiring in the hardware)

	0xFE00-0xFE9F: OAM - "Object Attribute Memory" - stores information about the sprites

	0xFEA0-0xFEFF: Unusable memory

	0xFF00-0xFF7F: MappedIO - holds controls values which are releated to I/O components

	0xFF80-0xFFFF: Zero Page - high speed RAM of 128 bytes, includes the Interrupt Enable Register
*/

#ifndef __GB_MMU_H
#define __GB_MMU_H

#include <string>
#include <vector>
#include "Cartridge/gb_cartridge.h"
#include "gb_utility.h"

class GBMMU
{
public:
	GBMMU(GBCartridge& i_Cartridge);
	virtual ~GBMMU() = default;
	GBMMU(const GBMMU&) = delete;
	GBMMU& operator=(const GBMMU&) = delete;
	
	byte Read(const WordAddress& i_Address) const;
	void Write(const WordAddress& i_Address, byte i_Value);

private:
	GBCartridge& m_Cartridge;
	vector<byte> m_VRAM = vector<byte>(0x9FFF - 0x8000 + 1);
	vector<byte> m_RAM = vector<byte>(0xFDFF - 0xC000 + 1); // including the shadow ram
	vector<byte> m_OAM = vector<byte>(0xFEFF - 0xFE00 + 1); // including the unusable section
	vector<byte> m_MappedIO = vector<byte>(0xFF7F - 0xFF00 + 1);
	vector<byte> m_ZeroPageRAM = vector<byte>(0xFFFF - 0xFF80 + 1);
};

#endif
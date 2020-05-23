/********************************************************************
 *			Copyright (C) 2020 Dedi Sidi			                *
 *														            *
 *			Cartridge class that is specific for the gameboy        *
 ********************************************************************/

#ifndef __GB_CARTRIDGE_
#define __GB_CARTRIDGE_

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include "../../Generic/logger.h"
#include "../../GB/CPU/gb_registers.h"
#include "../../GB/gb_utility.h"

class GBCartridge
{
public:
    GBCartridge(const std::string& i_RomFileName);
	virtual ~GBCartridge() = default;

    virtual byte Read(const WordAddress& i_Address) = 0;
    virtual void Write(const WordAddress& i_Address, byte i_Value) = 0;
};

/* cartridge with not additional banks memory banks (32K in size) */
class NoMBC : public GBCartridge
{
public:
    NoMBC(const std::string& i_RomFileName);
    virtual ~NoMBC() = default;

    virtual byte Read(const WordAddress& i_Address) = 0;
    virtual void Write(const WordAddress& i_Address, byte i_Value) = 0;

protected:
    vector<byte> m_ROMData;
};

#endif
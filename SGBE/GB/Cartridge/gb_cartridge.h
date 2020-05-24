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
#include "‏‏gb_cartridge_header.h"

class GBCartridge
{
public:
    GBCartridge(vector<byte>& i_ROMData);
	virtual ~GBCartridge() = default;

    virtual byte Read(const WordAddress& i_Address) = 0;
    virtual void Write(const WordAddress& i_Address, byte i_Value) = 0;

protected:
    vector<byte>& m_ROMData;
};

/* cartridge with not additional banks, no additional ROM and no additional RAM (32K in size) */
class NoMBC : public GBCartridge
{
public:
    NoMBC(vector<byte>& i_ROMData);
    virtual ~NoMBC() = default;

    virtual byte Read(const WordAddress& i_Address) override;
    virtual void Write(const WordAddress& i_Address, byte i_Value) override;
};

/* Cartridge Factory */

class CartridgeFactory
{
public:
    static GBCartridge* CreateCartridge(vector<byte>& i_ROMData, GBCartridgeHeader::CartridgeType_E i_CartridgeType);
};

#endif
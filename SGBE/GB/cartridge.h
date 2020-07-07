/********************************************************************
 *			Created by: Dedi Sidi, 2020 			                *
 *														            *
 *			Cartridge class that is specific for the gameboy        *
 ********************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include "cartridge_header.h"

#ifndef __CARTRIDGE_H
#define __CARTRIDGE_H

class Cartridge
{
public:
    Cartridge(vector<byte>& i_ROMData);
	virtual ~Cartridge() = default;

    virtual byte Read(const WordAddress& i_Address) = 0;
    virtual void Write(const WordAddress& i_Address, byte i_Value) = 0;

protected:
    vector<byte>& m_ROMData;
};

/* cartridge with not additional banks, no additional ROM and no additional RAM (32K in size) */
class NoMBC : public Cartridge
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
    static Cartridge* CreateCartridge(vector<byte>& i_ROMData, CartridgeHeader::CartridgeType_E i_CartridgeType);
};

#endif
/********************************************************************
 *			Created by: Dedi Sidi, 2020 			                *
 *														            *
 *			Cartridge class that is specific for the gameboy        *
 ********************************************************************/

#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "cartridge_header.h"

/* most of the info were taken from: * 
 *    https://gbdev.io/pandocs/      */

const word ROM_BANK_SIZE = 0x4000; // 16KB
const word RAM_BANK_SIZE = 0x2000; // 8KB

class Cartridge
{
public:
    Cartridge(vector<byte>& i_ROMData, const CartridgeHeader& i_CartridgeHeader);
	virtual ~Cartridge() = default;

    virtual byte Read(word i_Address) const = 0;
    virtual void Write(word i_Address, byte i_Value) = 0;

protected:
    const CartridgeHeader& m_CartridgeHeader;
    vector<byte>& m_ROMData;
};

/* cartridge with not additional banks, no additional ROM and no additional RAM (32K in size) */
class NoMBC : public Cartridge
{
public:
    NoMBC(vector<byte>& i_ROMData, const CartridgeHeader& i_CartridgeHeader);
    virtual ~NoMBC() = default;

    virtual byte Read(word i_Address) const override;
    virtual void Write(word i_Address, byte i_Value) override;
};

/* cartridge with additional banks */
/* max 2MByte ROM and/or 32KByte RAM */
class MBC1 : public Cartridge
{
public:
    MBC1(vector<byte>& i_ROMData, const CartridgeHeader& i_CartridgeHeader);
    virtual ~MBC1() = default;

    virtual byte Read(word i_Address) const override;
    virtual void Write(word i_Address, byte i_Value) override;

    const word MAX_RAM_SIZE = 4 * RAM_BANK_SIZE;

    enum class Banking_Mode
    {
        Rom_Banks_Only,
        Rom_And_Ram_Banks,
    };

private:
    /* read / write values methods */
    byte readFromSwitchableROMBanks(word i_Address) const;
    byte readFromSwitchableRAMBanks(word i_Address) const;
    void writeToRAMBanks(word i_Address, byte i_Value);

    /* control methods */
    void setRAMAccess(byte i_Value);
    void setROMBank(byte i_Value);
    void setRAMBankNumberOrUpperBitsOfROMBank(byte i_Value);
    void setMode(byte i_Value);

private:
    vector<byte> m_RAMBanks = vector<byte>(MAX_RAM_SIZE, 0);
    bool m_IsRAMEnabled;
    Banking_Mode m_Mode;
    byte m_RomBankNumber;
    byte m_HigherROMBankNumBits;
    byte m_RamBankNumber;
};

/* Cartridge Factory */
class CartridgeFactory
{
public:
    static Cartridge* CreateCartridge(vector<byte>& i_ROMData, const CartridgeHeader& i_CartridgeHeader);
};
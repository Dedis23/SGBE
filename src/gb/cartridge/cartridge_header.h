/********************************************************************
 *			Created by: Dedi Sidi, 2020 			                *
 *														            *
 *			A class that holds metadata about the cartridge         *
 ********************************************************************/

#pragma once
#include <vector>
#include <string>
#include "../utility.h"

/* Most of the info was taken from: */
/* https://gbdev.gg8.se/wiki/articles/The_Cartridge_Header */
/* https://gbdev.gg8.se/wiki/articles/Memory_Bank_Controllers */
/* I haven't implement everything here, only what is currently essential */

/* Cartridge metadata is held within addresses: 0x0100 - 0x014F  */

const word ENTRY_POINT_START = 0x100; // 4 bytes
const word ENTRY_POINT_END = 0x103;
const word LOGO_START = 0x104; // 48 bytes
const word LOGO_END = 0x133;
const word TITLE_START = 0x134; // 11 bytes
const word TITLE_END = 0x13E;
const word MANUFACTURER_CODE_START = 0x13F; // 4 bytes
const word MANUFACTURER_CODE_END = 0x142;
const word CGB_FLAG = 0x143; // 1 byte
const word NEW_LICENSE_CODE_START = 0x144; // 2 bytes
const word NEW_LICENSE_CODE_END = 0x145;
const word SGB_FLAG = 0x146; // 1 byte
const word CARTRIDGE_TYPE = 0x147; // 1 byte
const word ROM_BANKS_SIZE = 0x148; // 1 byte
const word RAM_BANKS_SIZE = 0x149; // 1 byte
const word DESTINATION_CODE = 0x14A; // 1 byte
const word OLD_LICENSE_CODE = 0x14B; // 1 byte
const word ROM_VERSION = 0x14C; // 1 byte
const word HEADER_CHECKSUM = 0x14D; // 1 byte
const word GLOBAL_CHECKSUM_START = 0x14E; // 2 bytes
const word GLOBAL_CHECKSUM_END = 0x14F;

class CartridgeHeader
{
public:
    enum class CartridgeType_E
    {
        NoMBC,
        MBC1,
        MBC2,
        MBC3,
        MBC5,
        UnknownCartridgeType,
    };

    enum class ROMSize_E
    {
        _32KB,
        _64KB,
        _128KB,
        _256KB,
        _512KB,
        _1MB,
        _2MB,
        _4MB,
        _8MB,
        _1_1MB,
        _1_2MB,
        _1_5MB,
        UnknownROMSize,
    };

    enum class RAMSize_E
    {
       None,
       _2KB,
       _8KB,
       _32KB,
       _64KB,
       _128KB,
       UnknownRAMSize,
    };


public:
    CartridgeHeader(const vector<byte>& i_ROMData);
	virtual ~CartridgeHeader() = default;

    bool VerifyChecksum();
    std::string GetTitle() const { return m_Title; }
    uint32_t GetVersion() const { return m_Version; }
    CartridgeType_E GetCartridgeType() const { return m_CartridgeType; }
    uint32_t GetNumOfSwitchableROMBanks() const { return m_NumOfSwitchableRomBanks; }
    uint32_t GetNumOfSwitchableRAMBanks() const { return m_NumOfSwitchableRamBanks; }
    std::string GetCartridgeTypeAsString() const;
    std::string GetROMSizeAsString() const;
    std::string GetRAMSizeAsString() const;

private:
    void readTitle();
    void readCartridgeType();
    void readROMSize();
    void readRAMSize();

private:
    const vector<byte>& m_ROMData;
    std::string m_Title;
    CartridgeType_E m_CartridgeType;
    ROMSize_E m_ROMSizeE;
    uint32_t m_NumOfSwitchableRomBanks;
    RAMSize_E m_RAMSizeE;
    uint32_t m_NumOfSwitchableRamBanks;
    uint32_t m_Version;
};
#include "cartridge.h"

Cartridge::Cartridge(vector<byte>& i_ROMData, const CartridgeHeader& i_CartridgeHeader) : m_CartridgeHeader(i_CartridgeHeader), m_ROMData(i_ROMData) {}

NoMBC::NoMBC(vector<byte>& i_ROMData, const CartridgeHeader& i_CartridgeHeader) : Cartridge(i_ROMData, i_CartridgeHeader) {}

byte NoMBC::Read(word i_Address) const
{
    if (i_Address >= 0x0 && i_Address <= 0x7FFF)
    {
        return m_ROMData[i_Address];
    }
    else if (i_Address <= 0xBFFF)
    {
        return 0x00;
    }
    
    LOG_ERROR(true, return 0, "Attempting to read address: 0x" << std::hex << i_Address << " it is out of range of the cartridge banks");
}

void NoMBC::Write(word i_Address, byte i_Value)
{
    // shouldn't get here because NoMBC type has no RAM so we can't write anything!
    LOG_ERROR(true, NOP, "Attempting to write to address: 0x" << std::hex << i_Address << " in a NoMBC cartridge!");
}

MBC1::MBC1(vector<byte>& i_ROMData, const CartridgeHeader& i_CartridgeHeader) : Cartridge(i_ROMData, i_CartridgeHeader), m_IsRAMEnabled(false),
m_Mode(Banking_Mode::Rom_Banks_Only), m_RomBankNumber(1), m_HigherROMBankNumBits(0), m_RamBankNumber(0)
{
    // m_RomBankNumber is set to 1 by default because 0 rom bank is the base bank that every cartridge has, the switchables is from 1 and above
}

byte MBC1::Read(word i_Address) const
{
    /* read from the base rom bank (non switchable) */
    if (i_Address <= 0x3FFF)
    {
        return m_ROMData[i_Address];
    }
    /* read from the switchable ROM banks */
    else if (i_Address >= 0x4000 && i_Address <= 0x7FFF)
    {
        return readFromSwitchableROMBanks(i_Address);
    }
    /* read from switchable RAM banks */
    else if (i_Address >= 0xA000 && i_Address <= 0xBFFF)
    {
        return readFromSwitchableRAMBanks(i_Address);
    }

    LOG_ERROR(true, return 0, "Attempting to read address: 0x" << std::hex << i_Address << " it is out of range of the cartridge banks");
}

void MBC1::Write(word i_Address, byte i_Value)
{
    /* enable/disable RAM access in MBC */
    if (i_Address <= 0x1FFF)
    {
        setRAMAccess(i_Value);
    }
    /* set switchable ROM bank number, 5 bits only are relevant */
    else if (i_Address >= 0x2000 && i_Address <= 0x3FFF)
    {
        setROMBank(i_Value);
    }
    else if (i_Address >= 0x4000 && i_Address <= 0x5FFF)
    {
        setRAMBankNumberOrUpperBitsOfROMBank(i_Value);
    }
    else if (i_Address >= 0x6000 && i_Address <= 0x7FFF)
    {
        setMode(i_Value);
    }
    else if (i_Address >= 0xA000 && i_Address <= 0xBFFF)
    {
        writeToRAMBanks(i_Address, i_Address);
    }
}

inline byte MBC1::readFromSwitchableROMBanks(word i_Address) const
{
    word addressInTheROMBank = i_Address - 0x4000;
    uint32_t offsetOfCurrentROMBank = 0;
    if (m_RomBankNumber != 0) // 0 is the base 16KB in the cartridge and not a switchable bank
    {
        offsetOfCurrentROMBank = ROM_BANK_SIZE * m_RomBankNumber;
    }
    uint32_t offset = offsetOfCurrentROMBank + addressInTheROMBank;
    return m_ROMData[offset];
}

inline byte MBC1::readFromSwitchableRAMBanks(word i_Address) const
{
    if (m_IsRAMEnabled)
    {
        word addressInTheRAMBank = i_Address - 0xA000;
        uint32_t offsetOfCurrentRAMBank = RAM_BANK_SIZE * m_RamBankNumber;
        uint32_t offset = offsetOfCurrentRAMBank + addressInTheRAMBank;
        return m_RAMBanks[offset];
    }
    LOG_ERROR(true, NOP, "Attempting to read from disabled RAM");
    return 0xFF;
}

inline void MBC1::writeToRAMBanks(word i_Address, byte i_Value)
{
    if (m_IsRAMEnabled)
    {
        switch (m_Mode)
        {
        case MBC1::Banking_Mode::Rom_Banks_Only:
            m_RAMBanks[i_Address - 0xA000] = i_Value;
            break;
        case MBC1::Banking_Mode::Rom_And_Ram_Banks:
            word addressInTheRAMBank = i_Address - 0xA000;
            uint32_t offsetOfCurrentRAMBank = RAM_BANK_SIZE * m_RamBankNumber;
            uint32_t offset = offsetOfCurrentRAMBank + addressInTheRAMBank;
            m_RAMBanks[offset] = i_Value;
            break;
        }
    }
}

inline void MBC1::setRAMAccess(byte i_Value)
{
    // writing 0x0A in this address space enables RAM to r/w
    m_IsRAMEnabled = ((i_Value & 0x0F) == 0x0A) ? true : false;
}

inline void MBC1::setROMBank(byte i_Value)
{
    byte relevantData = i_Value & 0x1F;
    switch (m_Mode)
    {
    case MBC1::Banking_Mode::Rom_Banks_Only:
        m_RomBankNumber = (relevantData | (m_HigherROMBankNumBits << 5));
        break;
    case MBC1::Banking_Mode::Rom_And_Ram_Banks:
        m_RomBankNumber = relevantData;
        break;
    }

    // banks 0x00, 0x20, 0x40, 0x60 are inaccessible and the current should be incremented
    if (m_RomBankNumber == 0x00 || m_RomBankNumber == 0x20 ||
        m_RomBankNumber == 0x40 || m_RomBankNumber == 0x60)
    {
        m_RomBankNumber++;
    }
    m_RomBankNumber &= (m_CartridgeHeader.GetNumOfSwitchableROMBanks() - 1);
}

/*  this value holds 2 bits that will be the RAM bank number or *
 *  upper bits of the current ROM / RAM bank                    *
 *  this is based on the current mode                           */
inline void MBC1::setRAMBankNumberOrUpperBitsOfROMBank(byte i_Value)
{
    byte relevantData = i_Value & 0x3; // get only the 2 LSB's
    switch (m_Mode)
    {
    case MBC1::Banking_Mode::Rom_Banks_Only:
        {
            byte m_HigherROMBankNumBits = relevantData;
            m_RomBankNumber = (m_RomBankNumber & 0x1F) | (m_HigherROMBankNumBits << 5); // set bits 6 and 7 using the new data

            // banks 0x00, 0x20, 0x40, 0x60 are inaccessible and the current should be incremented
            if (m_RomBankNumber == 0x00 || m_RomBankNumber == 0x20 ||
                m_RomBankNumber == 0x40 || m_RomBankNumber == 0x60)
            {
                m_RomBankNumber++;
            }
            m_RomBankNumber &= (m_CartridgeHeader.GetNumOfSwitchableROMBanks() - 1);
        }
        break;
    case MBC1::Banking_Mode::Rom_And_Ram_Banks:
        {
            m_RamBankNumber = relevantData;
            m_RamBankNumber &= (m_CartridgeHeader.GetNumOfSwitchableRAMBanks() - 1);
        }
        break;
    }
}

inline void MBC1::setMode(byte i_Value)
{
    switch (i_Value)
    {
    case 0x00:
        m_Mode = Banking_Mode::Rom_Banks_Only;
        break;
    case 0x01:
        m_Mode = Banking_Mode::Rom_And_Ram_Banks;
        break;
    default:
        LOG_ERROR(true, NOP, "Unknown value: 0x" << std::hex << i_Value << " tried to be written in MBC1 mode");
        break;
    }
}
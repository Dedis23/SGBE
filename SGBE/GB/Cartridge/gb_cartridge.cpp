#include "gb_cartridge.h"

GBCartridge::GBCartridge(vector<byte>& i_ROMData) : m_ROMData(i_ROMData) {}

NoMBC::NoMBC(vector<byte>& i_ROMData) : GBCartridge(i_ROMData) {}

byte NoMBC::Read(const WordAddress& i_Address)
{
    if (i_Address.InRange(0x0, 0x7FFF))
    {
        return m_ROMData[i_Address.GetValue()];
    }
    LOG_ERROR(true, NOP, "Attempting to read address: 0x" << i_Address.GetValue() << " it is out of range of the cartridge banks");
}

void NoMBC::Write(const WordAddress& i_Address, byte i_Value)
{
    // shouldn't get here because NoMBC has no RAM so we can't write anything!
    LOG_ERROR(true, NOP, "Attempting to write to address: 0x" << i_Address.GetValue() << " in a NoMBC cartridge!");
}

// currently the emulator supports only type NoMBC (aka no additional ROM / RAM)
// todo - add more types
GBCartridge* CartridgeFactory::CreateCartridge(vector<byte>& i_ROMData, GBCartridgeHeader::CartridgeType_E i_CartridgeType)
{
    GBCartridge* cartridge = nullptr;

	switch (i_CartridgeType)
	{
    case GBCartridgeHeader::CartridgeType_E::NoMBC:
        cartridge = new NoMBC(i_ROMData);
        break;
	}

    return cartridge;
}
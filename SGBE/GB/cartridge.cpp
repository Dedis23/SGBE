#include "cartridge.h"

Cartridge::Cartridge(vector<byte>& i_ROMData) : m_ROMData(i_ROMData) {}

NoMBC::NoMBC(vector<byte>& i_ROMData) : Cartridge(i_ROMData) {}

byte NoMBC::Read(word i_Address)
{
    if (i_Address >= 0x0 && i_Address <= 0x7FFF)
    {
        return m_ROMData[i_Address];
    }
    LOG_ERROR(true, return 0, "Attempting to read address: 0x" << i_Address << " it is out of range of the cartridge banks");
}

void NoMBC::Write(word i_Address, byte i_Value)
{
    // shouldn't get here because NoMBC type has no RAM so we can't write anything!
    LOG_ERROR(true, NOP, "Attempting to write to address: 0x" << i_Address << " in a NoMBC cartridge!");
}

// currently the emulator supports only type NoMBC (aka no additional ROM / RAM)
// todo - add more types
Cartridge* CartridgeFactory::CreateCartridge(vector<byte>& i_ROMData, CartridgeHeader::CartridgeType_E i_CartridgeType)
{
    Cartridge* cartridge = nullptr;

	switch (i_CartridgeType)
	{
    case CartridgeHeader::CartridgeType_E::NoMBC:
        cartridge = new NoMBC(i_ROMData);
        break;
	}

    return cartridge;
}
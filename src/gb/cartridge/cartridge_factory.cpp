#include "cartridge_factory.h"

Cartridge* CartridgeFactory::CreateCartridge(vector<byte>& i_ROMData, const CartridgeHeader& i_CartridgeHeader)
{
    Cartridge* cartridge = nullptr;

	switch (i_CartridgeHeader.GetCartridgeType())
	{
    case CartridgeHeader::CartridgeType_E::NoMBC:
        cartridge = new NoMBC(i_ROMData, i_CartridgeHeader);
        break;
    case CartridgeHeader::CartridgeType_E::MBC1:
        cartridge = new MBC1(i_ROMData, i_CartridgeHeader);
        break;
    case CartridgeHeader::CartridgeType_E::MBC2:
    case CartridgeHeader::CartridgeType_E::MBC3:
    case CartridgeHeader::CartridgeType_E::MBC5:
    case CartridgeHeader::CartridgeType_E::UnknownCartridgeType:
        LOG_ERROR(true, return nullptr, "Failed to create cartridge - unsupported cartridge type");
        break;
	}

    return cartridge;
}
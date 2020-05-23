#include "þþgb_cartridge_header.h"

GBCartridgeHeader::GBCartridgeHeader(const vector<byte>& i_ROMData) :
	m_ROMData(i_ROMData),
	CartridgeType(CartridgeType::UnknownCartridgeType),
	ROMSize(ROMSize::UnknownROMSize),
	RAMSize(RAMSize::UnknownRAMSize)
{
	readTitle();
	readCartridgeType();
	readROMSize();
	readRAMSize();
	Version	= m_ROMData[ROM_VERSION];
}

std::string GBCartridgeHeader::CartridgeTypeToString()
{
	switch (CartridgeType)
	{
		case CartridgeType::NoMBC:
			return "NoMBC";
		case CartridgeType::MBC1:
			return "MBC1";
		case CartridgeType::MBC2:
			return "MBC2";
		case CartridgeType::MBC3:
			return "MBC3";
		case CartridgeType::MBC5:
			return "MBC5";
		case CartridgeType::UnknownCartridgeType:
		default:
			return "Unknown cartridge type";
	}
}

std::string GBCartridgeHeader::ROMSizeToString()
{
	switch (ROMSize)
	{
		case ROMSize::_32KB:
			return "32KB";
		case ROMSize::_64KB:
			return "64KB";
		case ROMSize::_128KB:
			return "128KB";
		case ROMSize::_256KB:
			return "256KB";
		case ROMSize::_512KB:
			return "512KB";
		case ROMSize::_1MB:
			return "1MB";
		case ROMSize::_2MB:
			return "2MB";
		case ROMSize::_4MB:
			return "4MB";
		case ROMSize::_8MB:
			return "8MB";
		case ROMSize::_1_1MB:
			return "1.1MB";
		case ROMSize::_1_2MB:
			return "1.2MB";
		case ROMSize::_1_5MB:
			return "1.5MB";
		case ROMSize::UnknownROMSize:
		default:
			return "Unknown ROM size";
	}
}

std::string GBCartridgeHeader::RAMSizeToString()
{
	switch (RAMSize)
	{
		case RAMSize::None:
			return "None";
		case RAMSize::_2KB:
			return "2KB";
		case RAMSize::_8KB:
			return "8KB";
		case RAMSize::_32KB:
			return "32KB";
		case RAMSize::_64KB:
			return "64KB";
		case RAMSize::_128KB:
			return "128KB";
		case RAMSize::UnknownRAMSize:
		default:
			return "Unknown RAM size";
	}
}

void GBCartridgeHeader::readTitle()
{
	int pos = TITLE_START;
	for (int i = 0; i < (TITLE_END - TITLE_START  + 1); i++)
	{
		Title += static_cast<char>(m_ROMData[pos++]);
	}
}

void GBCartridgeHeader::readCartridgeType()
{
	switch (m_ROMData[CARTRIDGE_TYPE])
	{
		case 0x00:
		case 0x08:
		case 0x09:
			CartridgeType = CartridgeType::NoMBC;
			break;
		case 0x01:
		case 0x02:
		case 0x03:
			CartridgeType = CartridgeType::MBC1;
			break;
		case 0x05:
		case 0x06:
			CartridgeType = CartridgeType::MBC2;
			break;
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			CartridgeType = CartridgeType::MBC3;
			break;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			CartridgeType = CartridgeType::MBC5;
			break;
		default:
			CartridgeType = CartridgeType::UnknownCartridgeType;
			break;
	}
}

void GBCartridgeHeader::readROMSize()
{
	switch (m_ROMData[ROM_SIZE])
	{
		case 0x00:
			ROMSize = ROMSize::_32KB;
			break;
		case 0x01:
			ROMSize = ROMSize::_64KB;
			break;
		case 0x02:
			ROMSize = ROMSize::_128KB;
			break;
		case 0x03:
			ROMSize = ROMSize::_256KB;
			break;
		case 0x04:
			ROMSize = ROMSize::_512KB;
			break;
		case 0x05:
			ROMSize = ROMSize::_1MB;
			break;
		case 0x06:
			ROMSize = ROMSize::_2MB;
			break;
		case 0x07:
			ROMSize = ROMSize::_4MB;
			break;
		case 0x08:
			ROMSize = ROMSize::_8MB;
			break;
		case 0x52:
			ROMSize = ROMSize::_1_1MB;
			break;
		case 0x53:
			ROMSize = ROMSize::_1_2MB;
			break;
		case 0x54:
			ROMSize = ROMSize::_1_5MB;
			break;
		default:
			ROMSize = ROMSize::UnknownROMSize;
			break;
	}
}

void GBCartridgeHeader::readRAMSize()
{
	switch (m_ROMData[RAM_SIZE])
	{
	case 0x00:
		RAMSize = RAMSize::None;
		break;
	case 0x01:
		RAMSize = RAMSize::_2KB;
		break;
	case 0x02:
		RAMSize = RAMSize::_8KB;
		break;
	case 0x03:
		RAMSize = RAMSize::_32KB;
		break;
	case 0x04:
		RAMSize = RAMSize::_128KB;
		break;
	case 0x05:
		RAMSize = RAMSize::_64KB;
		break;
	default:
		RAMSize = RAMSize::UnknownRAMSize;
		break;
	}
}
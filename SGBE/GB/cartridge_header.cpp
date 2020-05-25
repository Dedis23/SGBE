#include "cartridge_header.h"

CartridgeHeader::CartridgeHeader(const vector<byte>& i_ROMData) :
	m_ROMData(i_ROMData),
	CartridgeType(CartridgeType_E::UnknownCartridgeType),
	ROMSize(ROMSize_E::UnknownROMSize),
	RAMSize(RAMSize_E::UnknownRAMSize)
{
	readTitle();
	readCartridgeType();
	readROMSize();
	readRAMSize();
	Version	= m_ROMData[ROM_VERSION];
}

bool CartridgeHeader::VerifyChecksum()
{
	// algorithm is: x=0:FOR i=0134h TO 014Ch:x=x-MEM[i]-1:NEXT
	// taken from https://gbdev.gg8.se/wiki/articles/The_Cartridge_Header

	uint32_t checksum = 0;
	for (word ptr = TITLE_START; ptr <= ROM_VERSION; ptr++)
	{
		checksum = checksum - m_ROMData[ptr] - 1;
	}
	return static_cast<byte>(checksum) == m_ROMData[HEADER_CHECKSUM];
}

std::string CartridgeHeader::CartridgeTypeToString()
{
	switch (CartridgeType)
	{
		case CartridgeType_E::NoMBC:
			return "NoMBC";
		case CartridgeType_E::MBC1:
			return "MBC1";
		case CartridgeType_E::MBC2:
			return "MBC2";
		case CartridgeType_E::MBC3:
			return "MBC3";
		case CartridgeType_E::MBC5:
			return "MBC5";
		case CartridgeType_E::UnknownCartridgeType:
		default:
			return "Unknown cartridge type";
	}
}

std::string CartridgeHeader::ROMSizeToString()
{
	switch (ROMSize)
	{
		case ROMSize_E::_32KB:
			return "32KB";
		case ROMSize_E::_64KB:
			return "64KB";
		case ROMSize_E::_128KB:
			return "128KB";
		case ROMSize_E::_256KB:
			return "256KB";
		case ROMSize_E::_512KB:
			return "512KB";
		case ROMSize_E::_1MB:
			return "1MB";
		case ROMSize_E::_2MB:
			return "2MB";
		case ROMSize_E::_4MB:
			return "4MB";
		case ROMSize_E::_8MB:
			return "8MB";
		case ROMSize_E::_1_1MB:
			return "1.1MB";
		case ROMSize_E::_1_2MB:
			return "1.2MB";
		case ROMSize_E::_1_5MB:
			return "1.5MB";
		case ROMSize_E::UnknownROMSize:
		default:
			return "Unknown ROM size";
	}
}

std::string CartridgeHeader::RAMSizeToString()
{
	switch (RAMSize)
	{
		case RAMSize_E::None:
			return "None";
		case RAMSize_E::_2KB:
			return "2KB";
		case RAMSize_E::_8KB:
			return "8KB";
		case RAMSize_E::_32KB:
			return "32KB";
		case RAMSize_E::_64KB:
			return "64KB";
		case RAMSize_E::_128KB:
			return "128KB";
		case RAMSize_E::UnknownRAMSize:
		default:
			return "Unknown RAM size";
	}
}

void CartridgeHeader::readTitle()
{
	int pos = TITLE_START;
	for (int i = 0; i < (TITLE_END - TITLE_START  + 1); i++)
	{
		Title += static_cast<char>(m_ROMData[pos++]);
	}
}

void CartridgeHeader::readCartridgeType()
{
	switch (m_ROMData[CARTRIDGE_TYPE])
	{
		case 0x00:
		case 0x08:
		case 0x09:
			CartridgeType = CartridgeType_E::NoMBC;
			break;
		case 0x01:
		case 0x02:
		case 0x03:
			CartridgeType = CartridgeType_E::MBC1;
			break;
		case 0x05:
		case 0x06:
			CartridgeType = CartridgeType_E::MBC2;
			break;
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			CartridgeType = CartridgeType_E::MBC3;
			break;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			CartridgeType = CartridgeType_E::MBC5;
			break;
		default:
			CartridgeType = CartridgeType_E::UnknownCartridgeType;
			break;
	}
}

void CartridgeHeader::readROMSize()
{
	switch (m_ROMData[ROM_SIZE])
	{
		case 0x00:
			ROMSize = ROMSize_E::_32KB;
			break;
		case 0x01:
			ROMSize = ROMSize_E::_64KB;
			break;
		case 0x02:
			ROMSize = ROMSize_E::_128KB;
			break;
		case 0x03:
			ROMSize = ROMSize_E::_256KB;
			break;
		case 0x04:
			ROMSize = ROMSize_E::_512KB;
			break;
		case 0x05:
			ROMSize = ROMSize_E::_1MB;
			break;
		case 0x06:
			ROMSize = ROMSize_E::_2MB;
			break;
		case 0x07:
			ROMSize = ROMSize_E::_4MB;
			break;
		case 0x08:
			ROMSize = ROMSize_E::_8MB;
			break;
		case 0x52:
			ROMSize = ROMSize_E::_1_1MB;
			break;
		case 0x53:
			ROMSize = ROMSize_E::_1_2MB;
			break;
		case 0x54:
			ROMSize = ROMSize_E::_1_5MB;
			break;
		default:
			ROMSize = ROMSize_E::UnknownROMSize;
			break;
	}
}

void CartridgeHeader::readRAMSize()
{
	switch (m_ROMData[RAM_SIZE])
	{
	case 0x00:
		RAMSize = RAMSize_E::None;
		break;
	case 0x01:
		RAMSize = RAMSize_E::_2KB;
		break;
	case 0x02:
		RAMSize = RAMSize_E::_8KB;
		break;
	case 0x03:
		RAMSize = RAMSize_E::_32KB;
		break;
	case 0x04:
		RAMSize = RAMSize_E::_128KB;
		break;
	case 0x05:
		RAMSize = RAMSize_E::_64KB;
		break;
	default:
		RAMSize = RAMSize_E::UnknownRAMSize;
		break;
	}
}
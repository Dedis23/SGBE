#include "cartridge_header.h"

CartridgeHeader::CartridgeHeader(const vector<byte>& i_ROMData) :
	m_ROMData(i_ROMData),
	m_CartridgeType(CartridgeType_E::UnknownCartridgeType),
	m_ROMSizeE(ROMSize_E::UnknownROMSize),
	m_NumOfSwitchableRomBanks(0),
	m_RAMSizeE(RAMSize_E::UnknownRAMSize),
	m_NumOfSwitchableRamBanks(0),
	m_Version(0)
{
	readTitle();
	readCartridgeType();
	readROMSize();
	readRAMSize();
	m_Version	= m_ROMData[ROM_VERSION];
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

std::string CartridgeHeader::GetCartridgeTypeAsString() const
{
	switch (m_CartridgeType)
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

std::string CartridgeHeader::GetROMSizeAsString() const
{
	switch (m_ROMSizeE)
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

std::string CartridgeHeader::GetRAMSizeAsString() const
{
	switch (m_RAMSizeE)
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
		m_Title += static_cast<char>(m_ROMData[pos++]);
	}
}

void CartridgeHeader::readCartridgeType()
{
	switch (m_ROMData[CARTRIDGE_TYPE])
	{
		case 0x00:
		case 0x08:
		case 0x09:
			m_CartridgeType = CartridgeType_E::NoMBC;
			break;
		case 0x01:
		case 0x02:
		case 0x03:
			m_CartridgeType = CartridgeType_E::MBC1;
			break;
		case 0x05:
		case 0x06:
			m_CartridgeType = CartridgeType_E::MBC2;
			break;
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			m_CartridgeType = CartridgeType_E::MBC3;
			break;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			m_CartridgeType = CartridgeType_E::MBC5;
			break;
		default:
			m_CartridgeType = CartridgeType_E::UnknownCartridgeType;
			break;
	}
}

void CartridgeHeader::readROMSize()
{
	switch (m_ROMData[ROM_BANKS_SIZE])
	{
		// note - each switchable rom bank is 16KB
		case 0x00:
			m_ROMSizeE = ROMSize_E::_32KB;
			m_NumOfSwitchableRomBanks = 2;
			break;
		case 0x01:
			m_ROMSizeE = ROMSize_E::_64KB;
			m_NumOfSwitchableRomBanks = 4;
			break;
		case 0x02:
			m_ROMSizeE = ROMSize_E::_128KB;
			m_NumOfSwitchableRomBanks = 8;
			break;
		case 0x03:
			m_ROMSizeE = ROMSize_E::_256KB;
			m_NumOfSwitchableRomBanks = 16;
			break;
		case 0x04:
			m_ROMSizeE = ROMSize_E::_512KB;
			m_NumOfSwitchableRomBanks = 32;
			break;
		case 0x05:
			m_ROMSizeE = ROMSize_E::_1MB;
			m_NumOfSwitchableRomBanks = 64;
			break;
		case 0x06:
			m_ROMSizeE = ROMSize_E::_2MB;
			m_NumOfSwitchableRomBanks = 128;
			break;
		case 0x07:
			m_ROMSizeE = ROMSize_E::_4MB;
			m_NumOfSwitchableRomBanks = 256;
			break;
		case 0x08:
			m_ROMSizeE = ROMSize_E::_8MB;
			m_NumOfSwitchableRomBanks = 512;
			break;
		case 0x52:
			m_ROMSizeE = ROMSize_E::_1_1MB;
			m_NumOfSwitchableRomBanks = 72;
			break;
		case 0x53:
			m_ROMSizeE = ROMSize_E::_1_2MB;
			m_NumOfSwitchableRomBanks = 80;
			break;
		case 0x54:
			m_ROMSizeE = ROMSize_E::_1_5MB;
			m_NumOfSwitchableRomBanks = 96;
			break;
		default:
			m_ROMSizeE = ROMSize_E::UnknownROMSize;
			break;
	}
}

void CartridgeHeader::readRAMSize()
{
	// note - each switchable ram bank is 8KB
	switch (m_ROMData[RAM_BANKS_SIZE])
	{
	case 0x00:
		m_RAMSizeE = RAMSize_E::None;
		m_NumOfSwitchableRamBanks = 0;
		break;
	case 0x01:
		m_RAMSizeE = RAMSize_E::_2KB;
		m_NumOfSwitchableRamBanks = 1;
		break;
	case 0x02:
		m_RAMSizeE = RAMSize_E::_8KB;
		m_NumOfSwitchableRamBanks = 1;
		break;
	case 0x03:
		m_RAMSizeE = RAMSize_E::_32KB;
		m_NumOfSwitchableRamBanks = 4;
		break;
	case 0x04:
		m_RAMSizeE = RAMSize_E::_128KB;
		m_NumOfSwitchableRamBanks = 16;
		break;
	case 0x05:
		m_RAMSizeE = RAMSize_E::_64KB;
		m_NumOfSwitchableRamBanks = 8;
		break;
	default:
		m_RAMSizeE = RAMSize_E::UnknownRAMSize;
		break;
	}
}
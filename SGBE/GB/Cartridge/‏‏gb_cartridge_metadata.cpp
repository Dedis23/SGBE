#include "þþgb_cartridge_metadata.h"

GBCartridgeMetaData::GBCartridgeMetaData(const vector<byte>& i_ROMData) :
	m_ROMData(i_ROMData),
	DestinationType(DestinationType::UnknownDestinationType),
	CartridgeType(CartridgeType::UnknownCartridgeType)
{

	extractTitle();
}

void GBCartridgeMetaData::extractTitle()
{
	// title is in 0x0134 - 0x0143h (16 bytes)
	for (int i = 0; i < 16; i++)
	{
		Title += static_cast<char>(m_ROMData[0x0134 + i]);
	}
}
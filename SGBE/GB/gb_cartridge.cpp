#include "gb_cartridge.h"

GBCartridge::GBCartridge(const string& i_RomFileName)
{
	bool res = false;

	res = loadROM(i_RomFileName);
	LOG_CRITICAL(res == false, throw exception(), "Failed to load ROM data");
}

bool GBCartridge::loadROM(const string& i_RomFileName)
{
	bool res = false;

	// read file
	std::ifstream romFile(i_RomFileName, std::ios::binary);
	LOG_ERROR(!romFile.is_open() || !romFile.good(), return false, "Failed to read the file: " << i_RomFileName);

	// get the size
	romFile.seekg(0, std::ios::end);
	std::streampos fileSize = romFile.tellg();
	romFile.seekg(0, std::ios::beg);

	// reserve capacity in the vector
	m_ROMData.reserve(fileSize);

	// read the data
	m_ROMData.insert(m_ROMData.begin(), istream_iterator<uint8_t>(romFile), std::istream_iterator<uint8_t>());
	LOG_INFO(true, NOP, "Read " << m_ROMData.size() / 1024 << "KB from file: " << i_RomFileName);
	romFile.close();

	return true;
}
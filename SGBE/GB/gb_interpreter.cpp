#include "gb_interpreter.h"

GBInterpreter::GBInterpreter() : m_MMU(*m_Cartridge), m_Cartridge(nullptr) {}

GBInterpreter::~GBInterpreter()
{
	delete m_Cartridge;
}

bool GBInterpreter::Initialize(const std::string& i_RomFileName)
{
	bool res = false;

	res = loadROM(i_RomFileName);
	LOG_CRITICAL(res == false, return false, "Failed to load ROM data");

	res = initializeCartridge();
	LOG_CRITICAL(res == false, return false, "Failed to initialzie cartridge");

	return true;
}

/* This is the main emulation loop */
void GBInterpreter::Run()
{
	//m_CPU.Step();
}

bool GBInterpreter::loadROM(const string& i_RomFileName)
{
	bool res = false;

	// read file
	std::ifstream romFile(i_RomFileName, std::ios::binary);
	LOG_ERROR(!romFile.is_open() || !romFile.good(), return false, "Failed to read the file: " << i_RomFileName);

	// get the size
	romFile.seekg(0, std::ios::end);
	std::streampos fileSize = romFile.tellg();
	romFile.seekg(0, std::ios::beg);

	// read the data - fill the stream into vector<char>
	vector<char> container(fileSize);
	romFile.read(&container[0], fileSize);
	// move to member vector of type byte
	std::move(std::begin(container), std::end(container), std::back_inserter(m_ROMData));
	LOG_INFO(true, NOP, "Read " << m_ROMData.size() / 1024 << "KB from file: " << i_RomFileName);
	romFile.close();

	return true;
}

bool GBInterpreter::initializeCartridge()
{
	bool res = false;

	GBCartridgeMetaData cartridgeMetaData(m_ROMData);

	return true;
}
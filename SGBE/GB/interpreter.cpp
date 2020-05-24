#include "interpreter.h"

Interpreter::Interpreter() : m_MMU(*m_Cartridge), m_Cartridge(nullptr) {}

Interpreter::~Interpreter()
{
	delete m_Cartridge;
}

bool Interpreter::Initialize(const std::string& i_RomFileName)
{
	bool res = false;

	res = loadROM(i_RomFileName);
	LOG_CRITICAL(res == false, return false, "Failed to load ROM data");

	res = initializeCartridge();
	LOG_CRITICAL(res == false, return false, "Failed to initialzie cartridge");

	return true;
}

bool Interpreter::IsCartridgeLoadedSuccessfully()
{
	return m_Cartridge != nullptr;
}

/* This is the main emulation loop */
void Interpreter::Run()
{
	//m_CPU.Step();
}

bool Interpreter::loadROM(const string& i_RomFileName)
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

bool Interpreter::initializeCartridge()
{
	bool res = false;

	CartridgeHeader cartridgeHeader(m_ROMData);
	LOG_INFO(true, NOP, "Cartridge title: " << cartridgeHeader.Title);
	LOG_INFO(true, NOP, "Version: " << cartridgeHeader.Version);
	LOG_INFO(true, NOP, "Cartridge type: " << cartridgeHeader.CartridgeTypeToString());
	LOG_INFO(true, NOP, "Total ROM size: " << cartridgeHeader.ROMSizeToString());
	LOG_INFO(true, NOP, "Additional RAM size: " << cartridgeHeader.RAMSizeToString());

	m_Cartridge = CartridgeFactory::CreateCartridge(m_ROMData, cartridgeHeader.CartridgeType);
	LOG_ERROR(m_Cartridge == nullptr, NOP, "Cartridge type " << cartridgeHeader.CartridgeTypeToString() << " is currently unsupported");

	return true;
}
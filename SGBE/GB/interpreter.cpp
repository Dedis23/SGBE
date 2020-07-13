#include "interpreter.h"

Interpreter::Interpreter(vector<byte>& i_ROMData) : m_ROMData(i_ROMData), m_CPU(nullptr),
m_MMU(nullptr), m_CartridgeHeader(nullptr), m_Cartridge(nullptr) {}

Interpreter::~Interpreter()
{
	delete m_CartridgeHeader;
	delete m_Cartridge;
	delete m_MMU;
	delete m_CPU;
	delete m_PPU;
}

bool Interpreter::Initialize()
{
	bool res = false;

	m_CartridgeHeader = new CartridgeHeader(m_ROMData);
	LOG_ERROR(m_CartridgeHeader == nullptr, return false, "Failed to read the cartridge header");

	res = m_CartridgeHeader->VerifyChecksum();
	LOG_ERROR(res == false, return true, "Failed to verify cartridge checksum");
	LOG_INFO(true, NOP, "Cartridge checksum verified successfully.");

	res = initializeCartridge();
	LOG_CRITICAL(res == false, return false, "Failed to initialzie cartridge");

	m_Timer = new Timer();
	LOG_ERROR(m_Timer == nullptr, return false, "Failed to initialize the Timer");

	m_PPU = new PPU();
	LOG_ERROR(m_PPU == nullptr, return false, "Failed to initalize the GPU");

	m_MMU = new MMU(*m_Cartridge, *m_Timer);
	LOG_ERROR(m_MMU == nullptr, return false, "Failed to initialize the MMU");
																    
	m_CPU = new CPU(*m_MMU);									    
	LOG_ERROR(m_CPU == nullptr, return false, "Failed to initialize the CPU");

	return true;
}

bool Interpreter::IsCartridgeLoadedSuccessfully()
{
	return m_Cartridge == nullptr ? false : true;
}

/* This is the main emulation loop */
void Interpreter::Run()
{
	while (true)
	{
		uint32_t cycles = 0;
		m_CPU->Step(cycles);
		m_Timer->Step(cycles);
	}
}

bool Interpreter::initializeCartridge()
{
	bool res = false;

	LOG_INFO(true, NOP, "Cartridge title: " << m_CartridgeHeader->Title);
	LOG_INFO(true, NOP, "Version: " << m_CartridgeHeader->Version);
	LOG_INFO(true, NOP, "Cartridge type: " << m_CartridgeHeader->CartridgeTypeToString());
	LOG_INFO(true, NOP, "Total ROM size: " << m_CartridgeHeader->ROMSizeToString());
	LOG_INFO(true, NOP, "Additional RAM size: " << m_CartridgeHeader->RAMSizeToString());

	m_Cartridge = CartridgeFactory::CreateCartridge(m_ROMData, m_CartridgeHeader->CartridgeType);
	LOG_ERROR(m_Cartridge == nullptr, NOP, "Cartridge type " << m_CartridgeHeader->CartridgeTypeToString() << " is unsupported");

	return true;
}
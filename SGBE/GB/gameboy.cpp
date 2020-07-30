#include "gameboy.h"

Gameboy::Gameboy(vector<byte>& i_ROMData, function<void(Pixel* i_FrameBuffer)> i_RenderFuncPtr) : m_ROMData(i_ROMData), m_MMU(nullptr), m_CPU(nullptr),
m_Timer(nullptr), m_GPU(nullptr), m_CartridgeHeader(nullptr), m_Cartridge(nullptr), m_RenderScreen(i_RenderFuncPtr) {}

Gameboy::~Gameboy()
{
	delete m_CartridgeHeader;
	delete m_Cartridge;
	delete m_MMU;
	delete m_CPU;
	delete m_GPU;
}

bool Gameboy::Initialize()
{
	bool res = false;

	res = initializeCartridge();
	LOG_CRITICAL(res == false, return false, "Failed to initialzie cartridge");

	m_MMU = new MMU(*this, *m_Cartridge);
	LOG_ERROR(m_MMU == nullptr, return false, "Failed to initialize the MMU");

	m_CPU = new CPU(*this, *m_MMU);
	LOG_ERROR(m_CPU == nullptr, return false, "Failed to initialize the CPU");

	m_Timer = new Timer(*this);
	LOG_ERROR(m_Timer == nullptr, return false, "Failed to initialize the Timer");

	m_GPU = new GPU(*this);
	LOG_ERROR(m_GPU == nullptr, return false, "Failed to initalize the GPU");

	return true;
}

bool Gameboy::IsCartridgeLoadedSuccessfully()
{
	return m_Cartridge == nullptr ? false : true;
}

/* This is the main emulation loop */
void Gameboy::Step()
{
	uint32_t currentFrameCycles = 0;
	while (currentFrameCycles < MAX_CYCLES_BEFORE_RENDERING)
	{
		m_CPU->Step(currentFrameCycles);
		m_Timer->Step(currentFrameCycles);
		m_GPU->Step(currentFrameCycles);
		m_CPU->HandleInterrupts();
	}
	Pixel testArr[GAMEBOY_SCREEN_HEIGHT * GAMEBOY_SCREEN_WIDTH];
	m_RenderScreen(testArr);
	//renderScreen();
}

CPU& Gameboy::GetCPU()
{
	return *m_CPU;
}

MMU& Gameboy::GetMMU()
{
	return *m_MMU;
}

Timer& Gameboy::GetTimer()
{
	return *m_Timer;
}

GPU& Gameboy::GetGPU()
{
	return *m_GPU;
}

bool Gameboy::initializeCartridge()
{
	bool res = false;

	m_CartridgeHeader = new CartridgeHeader(m_ROMData);
	LOG_ERROR(m_CartridgeHeader == nullptr, return false, "Failed to read the cartridge header");

	res = m_CartridgeHeader->VerifyChecksum();
	LOG_ERROR(res == false, return true, "Failed to verify cartridge checksum");
	LOG_INFO(true, NOP, "Cartridge checksum verified successfully.");

	LOG_INFO(true, NOP, "Cartridge title: " << m_CartridgeHeader->Title);
	LOG_INFO(true, NOP, "Version: " << m_CartridgeHeader->Version);
	LOG_INFO(true, NOP, "Cartridge type: " << m_CartridgeHeader->CartridgeTypeToString());
	LOG_INFO(true, NOP, "Total ROM size: " << m_CartridgeHeader->ROMSizeToString());
	LOG_INFO(true, NOP, "Additional RAM size: " << m_CartridgeHeader->RAMSizeToString());

	m_Cartridge = CartridgeFactory::CreateCartridge(m_ROMData, m_CartridgeHeader->CartridgeType);
	LOG_ERROR(m_Cartridge == nullptr, NOP, "Cartridge type " << m_CartridgeHeader->CartridgeTypeToString() << " is unsupported");

	return true;
}
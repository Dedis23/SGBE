#include "gameboy.h"

Gameboy::Gameboy(vector<byte>& i_ROMData, function<void(const Pixel* i_FrameBuffer)> i_RenderFuncPtr) : m_ROMData(i_ROMData),  m_RenderScreen(i_RenderFuncPtr), m_CartridgeHeader(nullptr), m_Cartridge(nullptr),
m_MMU(nullptr), m_CPU(nullptr), m_GPU(nullptr), m_Timer(nullptr), m_Joypad(nullptr) {}

Gameboy::~Gameboy()
{
	delete m_CartridgeHeader;
	delete m_Cartridge;
	delete m_MMU;
	delete m_CPU;
	delete m_GPU;
	delete m_Timer;
	delete m_Joypad;
}

bool Gameboy::Initialize()
{
	bool res = false;

	res = initializeCartridge();
	LOG_CRITICAL(res == false, return false, "Failed to initialzie cartridge");

	m_MMU = new MMU(m_GBInternals, *m_Cartridge);
	LOG_ERROR(m_MMU == nullptr, return false, "Failed to initialize the MMU");
	m_GBInternals.AttachMMU(m_MMU);

	m_CPU = new CPU(m_GBInternals, *m_MMU);
	LOG_ERROR(m_CPU == nullptr, return false, "Failed to initialize the CPU");
	m_GBInternals.AttachCPU(m_CPU);

	m_GPU = new GPU(m_GBInternals);
	LOG_ERROR(m_GPU == nullptr, return false, "Failed to initalize the GPU");
	m_GBInternals.AttachGPU(m_GPU);

	m_Timer = new Timer(m_GBInternals);
	LOG_ERROR(m_Timer == nullptr, return false, "Failed to initialize the Timer");
	m_GBInternals.AttachTimer(m_Timer);

	m_Joypad = new Joypad(m_GBInternals);
	LOG_ERROR(m_Joypad == nullptr, return false, "Failed to initalize the Joypad");
	m_GBInternals.AttachJoypad(m_Joypad);

	return true;
}

bool Gameboy::IsCartridgeLoadedSuccessfully()
{
	return m_Cartridge == nullptr ? false : true;
}

/* step a single frame in the Gameboy */
void Gameboy::Step()
{
	uint32_t currentFrameCycles = 0;

	// let the cpu run and do enough cycles before rendering the frame
	while (currentFrameCycles < MAX_CYCLES_BEFORE_RENDERING)
	{
		uint32_t cyclesCurrentOperation = 0;
		m_CPU->Step(cyclesCurrentOperation);
		m_Timer->Step(cyclesCurrentOperation);
		m_GPU->Step(cyclesCurrentOperation);
		m_CPU->HandleInterrupts(cyclesCurrentOperation);
		currentFrameCycles += cyclesCurrentOperation;
	}
	// render
	m_RenderScreen(m_GPU->GetFrameBuffer());
}

void Gameboy::KeyPressed(const GBButtons& i_PressedButton) const
{
	m_Joypad->KeyPressed(i_PressedButton);
}

void Gameboy::KeyReleased(const GBButtons& i_ReleasedButton) const
{
	m_Joypad->KeyReleased(i_ReleasedButton);
}

void Gameboy::ChangePalette(const Palette& i_Palette)
{
	m_GPU->ChangePalette(i_Palette);
}

void Gameboy::ChangeToNextPalette()
{
	m_GPU->ChangeToNextPalette();
}

bool Gameboy::initializeCartridge()
{
	bool res = false;

	// before we create the cartridge, we first need to create a cartridge header class that holds metadata about the cartridge
	// once we have the cartridge metadata, we will know which cartridge type and request it from the factory

	m_CartridgeHeader = new CartridgeHeader(m_ROMData);
	LOG_ERROR(m_CartridgeHeader == nullptr, return false, "Failed to read the cartridge header");

	res = m_CartridgeHeader->VerifyChecksum();
	LOG_ERROR(res == false, return true, "Failed to verify cartridge checksum");
	LOG_INFO(true, NOP, "Cartridge checksum verified successfully.");

	LOG_INFO(true, NOP, "Cartridge title: " << m_CartridgeHeader->GetTitle());
	LOG_INFO(true, NOP, "Version: " << m_CartridgeHeader->GetVersion());
	LOG_INFO(true, NOP, "Cartridge type: " << m_CartridgeHeader->GetCartridgeTypeAsString());
	LOG_INFO(true, NOP, "Total ROM size: " << m_CartridgeHeader->GetROMSizeAsString());
	LOG_INFO(true, NOP, "Additional RAM size: " << m_CartridgeHeader->GetRAMSizeAsString());

	m_Cartridge = CartridgeFactory::CreateCartridge(m_ROMData, *m_CartridgeHeader);
	LOG_ERROR(m_Cartridge == nullptr, NOP, "Cartridge type " << m_CartridgeHeader->GetCartridgeTypeAsString() << " is unsupported!");

	return true;
}
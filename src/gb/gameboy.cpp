#include "gameboy.h"

Gameboy::Gameboy(vector<byte>& i_ROMData, function<void(const Pixel* i_FrameBuffer)> i_RenderFuncPtr) : m_ROMData(i_ROMData), m_MMU(nullptr), m_CPU(nullptr),
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
	static int frameNum = 1;
	uint32_t commandNum = 1;
	bool write = false;
	std::ofstream testFile;
	if (frameNum == 1)
	{
		testFile = std::ofstream("checkFrameCycles_SGBE.txt", std::ios_base::trunc);
	}
	else
	{
		testFile = std::ofstream("checkFrameCycles_SGBE.txt", std::ios_base::app);
	}	
	if (write)
	{

		if (!testFile.is_open())
		{
			//exit(1);
			std::cout << "should exit here" << std::endl;
		}
	}
	else
	{
		testFile.close();
	}
	uint32_t currentFrameCycles = 0;
	while (currentFrameCycles < MAX_CYCLES_BEFORE_RENDERING)
	{
		static std::string lastComTest;

		//if (frameNum == 4 && commandNum == 0x1866)
		//{
		//	std::cout << "STOP!" << std::endl;
		//}

		uint32_t cyclesCurrentOperation = 0;
		std::string last_command = m_CPU->Step(cyclesCurrentOperation);
		lastComTest = last_command;
		m_Timer->Step(cyclesCurrentOperation);
		m_GPU->Step(write, testFile, cyclesCurrentOperation);
		m_CPU->HandleInterrupts(cyclesCurrentOperation);
		currentFrameCycles += cyclesCurrentOperation;
		if (write)
		{
			testFile << "Frame number: " << frameNum << std::endl
			<< "Command number: " << commandNum++ << " - " << last_command
			<< " took: " << cyclesCurrentOperation << std::endl
			<< "Overall cycles: " << currentFrameCycles << std::endl;
			if (write)
			{
				m_CPU->dumpRegisters(testFile);
			}
		}
		else
		{
			commandNum++;
		}
	}
	m_RenderScreen(m_GPU->GetFrameBuffer());
	if (write)
	{
		std::cout << "done writing full frame number: " << frameNum << " to the file" << std::endl;
		frameNum++;
		if (frameNum == 11)
		{ 
			testFile.close();
			exit(0);
		}
	}
	else
	{
		frameNum++;
	}
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

	LOG_INFO(true, NOP, "Cartridge title: " << m_CartridgeHeader->GetTitle());
	LOG_INFO(true, NOP, "Version: " << m_CartridgeHeader->GetVersion());
	LOG_INFO(true, NOP, "Cartridge type: " << m_CartridgeHeader->GetCartridgeTypeAsString());
	LOG_INFO(true, NOP, "Total ROM size: " << m_CartridgeHeader->GetROMSizeAsString());
	LOG_INFO(true, NOP, "Additional RAM size: " << m_CartridgeHeader->GetRAMSizeAsString());

	m_Cartridge = CartridgeFactory::CreateCartridge(m_ROMData, *m_CartridgeHeader);
	LOG_ERROR(m_Cartridge == nullptr, NOP, "Cartridge type " << m_CartridgeHeader->GetCartridgeTypeAsString() << " is unsupported");

	return true;
}
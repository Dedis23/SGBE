#include "sgbe.h"

string SGBE::s_ROMFileName = "";

SGBE::SGBE() : m_ShouldExist(false), m_SDLWrapper(nullptr), m_Gameboy(nullptr) {}

SGBE::~SGBE()
{
	delete m_Gameboy;
	delete m_SDLWrapper;
	
	// clean logger instance
	Logger::ResetInstance();
}

bool SGBE::Initialize(int argc, char* argv[])
{
	bool res = false;

	res = loadDefaultSettings();
	LOG_CRITICAL(res == false, return false, "Failed to load default settings");

	res = loadArguments(argc, argv);
	LOG_CRITICAL(res == false, return false, "Failed to load arguments");

	// assert that the user inserted a rom file name
	LOG_ERROR(s_ROMFileName == "", return false, "Cannot initialize without a ROM file name");

	res = loadROM(s_ROMFileName);
	LOG_CRITICAL(res == false, return false, "Failed to load ROM data");

	// initialize SDL wrapper
	m_SDLWrapper = new SDLWrapper();
	LOG_CRITICAL(m_SDLWrapper == nullptr, return false, "Failed to allocate memory for the SDL wrapper");

	res = m_SDLWrapper->Initialize("SGBE", GAMEBOY_SCREEN_WIDTH, GAMEBOY_SCREEN_HEIGHT);
	LOG_CRITICAL(res == false, return false, "Failed to initialize the SDL wrapper");

	// initialize the gameboy
	using std::placeholders::_1;
	function<void(const Pixel*)> renderScreenPtr = std::bind(&SDLWrapper::RenderScreen, m_SDLWrapper, _1);

	m_Gameboy = new Gameboy(m_ROMData, renderScreenPtr);
	LOG_CRITICAL(m_Gameboy == nullptr , return false, "Failed to allocate memory for the interpreter");

	res = m_Gameboy->Initialize();
	LOG_CRITICAL(res == false, return false, "Failed to initialize the interpreter");

	res = m_Gameboy->IsCartridgeLoadedSuccessfully();
	LOG_INFO(res == true, NOP, "SGBE initialized successfully.");

	return true;
}

void SGBE::Run()
{
	chrono::time_point<chrono::high_resolution_clock> singleFrameStartTime, singleFrameEndTime;
	double timeForASingleFrame = 1000 / TARGET_FPS; // time for a single frame in miliseconds
	chrono::duration<double, milli> elapsedSingleFrameTime;

	// main loop
	while (!m_ShouldExist)
	{
		// handle input
		handleInput();

		 // step a single frame
		singleFrameStartTime = chrono::high_resolution_clock::now();
		m_Gameboy->Step();
		singleFrameEndTime = chrono::high_resolution_clock::now();
		elapsedSingleFrameTime = chrono::duration_cast<chrono::duration<double, milli>> (singleFrameEndTime - singleFrameStartTime);
		
		// frame throttle - do a thread sleep if the frame took less time than expected
		if (elapsedSingleFrameTime.count() < timeForASingleFrame)
		{
			this_thread::sleep_for(chrono::duration<double, milli>(timeForASingleFrame - elapsedSingleFrameTime.count()));
		}
	}
	
	exitSGBE();
}

bool SGBE::loadDefaultSettings()
{
	// set default logger settings
	LOGGER_SET_LOG_LEVEL(Logger::LogLevel::Info);
	LOGGER_SET_LOG_TYPE(Logger::LogType::Console);
	LOGGER_SET_LOG_METADATA(false);

	return true;
}

bool SGBE::loadArguments(int argc, char* argv[])
{
	CLI cli;
	cli.AddOption("game", &cliRomOption);
	cli.AddOption("silent", &cliSilentOption);
	cli.AddOption("logfile", &cliLogFileNameOption);
	cli.LoadArgs(argc, argv);

	return true;
}

bool SGBE::loadROM(const string& i_RomFileName)
{
	// read file
	std::ifstream romFile(i_RomFileName, std::ios::binary);
	LOG_ERROR(!romFile.is_open() || !romFile.good(), return false, "Failed to read the file: " << i_RomFileName);

	// get the size
	romFile.seekg(0, std::ios::end);
	std::streampos fileSize = romFile.tellg();
	romFile.seekg(0, std::ios::beg);

	// read the data - fill the stream into vector<byte>
	vector<byte> container(fileSize);
	romFile.read(&container[0], fileSize);

	// move to member vector of type byte
	std::move(std::begin(container), std::end(container), std::back_inserter(m_ROMData));
	LOG_INFO(true, NOP, "Read " << m_ROMData.size() / 1024 << "KB from file: " << i_RomFileName);
	romFile.close();

	return true;
}

void SGBE::cliRomOption(const string& i_RomFileName)
{
	if (i_RomFileName.size() <= 0)
	{
		LOG_ERROR(true, return, "Rom file name is empty");
	}
	else
	{
		s_ROMFileName = i_RomFileName;	
	}	
}

void SGBE::cliSilentOption()
{
	LOGGER_SET_LOG_LEVEL(Logger::LogLevel::Disabled);
}

void SGBE::cliLogFileNameOption(const string& i_LogFileName)
{
	LOGGER_SET_LOG_TYPE(Logger::LogType::File);
	LOG_INFO(true, NOP, "Log type set to File");

	LOGGER_SET_FILE_NAME(i_LogFileName);
	LOG_INFO(true, NOP, "Log file name set to" << i_LogFileName);
}

void SGBE::handleInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
				case SDLK_x:
					m_Gameboy->KeyPressed(GBButtons::A);
					break;
				case SDLK_z:
					m_Gameboy->KeyPressed(GBButtons::B);
					break;
				case SDLK_RETURN:
					m_Gameboy->KeyPressed(GBButtons::Start);
					break;
				case SDLK_RSHIFT:
					m_Gameboy->KeyPressed(GBButtons::Select);
					break;
				case SDLK_RIGHT:
					m_Gameboy->KeyPressed(GBButtons::Right);
					break;
				case SDLK_LEFT:
					m_Gameboy->KeyPressed(GBButtons::Left);
					break;
				case SDLK_UP:
					m_Gameboy->KeyPressed(GBButtons::Up);
					break;
				case SDLK_DOWN:
					m_Gameboy->KeyPressed(GBButtons::Down);
					break;
				case SDLK_F1:
					m_Gameboy->ChangeToNextPalette();
					break;
			}
		}

		if (event.type == SDL_KEYUP)
		{
			switch (event.key.keysym.sym)
			{
				case SDLK_x:
					m_Gameboy->KeyReleased(GBButtons::A);
					break;
				case SDLK_z:
					m_Gameboy->KeyReleased(GBButtons::B);
					break;
				case SDLK_RETURN:
					m_Gameboy->KeyReleased(GBButtons::Start);
					break;
				case SDLK_RSHIFT:
					m_Gameboy->KeyReleased(GBButtons::Select);
					break;
				case SDLK_RIGHT:
					m_Gameboy->KeyReleased(GBButtons::Right);
					break;
				case SDLK_LEFT:
					m_Gameboy->KeyReleased(GBButtons::Left);
					break;
				case SDLK_UP:
					m_Gameboy->KeyReleased(GBButtons::Up);
					break;
				case SDLK_DOWN:
					m_Gameboy->KeyReleased(GBButtons::Down);
					break;
			}
		}
		
		if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
		{
			m_ShouldExist = true;
		}
	}
}

void SGBE::exitSGBE()
{
	// TODO, add save before exit
	exit(EXIT_SUCCESS);
}
#include "sgbe.h"

string SGBE::s_ROMFileName = "";

SGBE::SGBE() : m_SDLWrapper(nullptr), m_Gameboy(nullptr) {}

SGBE::~SGBE()
{
	// delete interpreter
	delete m_Gameboy;
	m_Gameboy = nullptr;

	// delete SDL wrapper
	delete m_SDLWrapper;
	m_SDLWrapper = nullptr;
	
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

	LOG_INFO(true, NOP, "SGBE initialized successfully." << endl);

	return true;
}

void SGBE::Run()
{
	chrono::time_point<chrono::high_resolution_clock> testStart, testEnd; // delete this later
	testStart = chrono::high_resolution_clock::now();
	chrono::time_point<chrono::high_resolution_clock> startFrameTime, endFrameTime, elapsedFrameTime;
	if (m_Gameboy->IsCartridgeLoadedSuccessfully())
	{
		SDL_Event event;
		while (true) // change this with SDL quit check i.e press X or click ESC
		{
			// TODO - move this into the SDLWrapper
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
				{
					GBButtons button = GBButtons::None;
					switch (event.key.keysym.sym)
					{
						case SDLK_x:
								button = GBButtons::A;
							break;
						case SDLK_z:
								button = GBButtons::B;
							break;
						case SDLK_RETURN:
								button = GBButtons::Start;
							break;
						case SDLK_RSHIFT:
								button = GBButtons::Select;
							break;
						case SDLK_RIGHT:
								button = GBButtons::Right;
							break;
						case SDLK_LEFT:
								button = GBButtons::Left;
							break;
						case SDLK_UP:
								button = GBButtons::Up;
							break;
						case SDLK_DOWN:
								button = GBButtons::Down;
							break;
					}

					if (event.type == SDL_KEYDOWN)
					{
						m_Gameboy->KeyPressed(button);
					}
					else
					{
						m_Gameboy->KeyReleased(button);
					}
				}

				if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
				{
					exitSGBE();
				}
			}

			startFrameTime = chrono::high_resolution_clock::now();
			// main loop here
			m_Gameboy->Step(); // step a single frame
			endFrameTime = chrono::high_resolution_clock::now();
			static int test = 1;
			test++;
			if (test > 60)
			{
				testEnd = chrono::high_resolution_clock::now();
				auto testElapsed = chrono::duration_cast<chrono::duration<float, milli>> (testEnd - testStart);
				//cout << "Overall 60 frames took: " << testElapsed.count() << " milliseconds" << endl;
				test %= 60;
				testStart = chrono::high_resolution_clock::now();
			}
		}
	}
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
	bool res = false;

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
	s_ROMFileName = i_RomFileName;
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

void SGBE::exitSGBE()
{
	// TODO, add save before exit
	exit(EXIT_SUCCESS);
}
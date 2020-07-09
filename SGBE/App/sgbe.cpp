#include "sgbe.h"

string SGBE::s_ROMFileName = "";

SGBE::SGBE() : m_Window(nullptr), m_Renderer(nullptr), m_Interpreter(nullptr) {}

SGBE::~SGBE()
{
	// delete interpreter
	delete m_Interpreter;
	m_Interpreter = nullptr;
	
	// clean logger instance
	Logger::ResetInstance();

	// clean SDL
	SDL_DestroyRenderer(m_Renderer);
	SDL_DestroyWindow(m_Window);
	SDL_Quit();
}

bool SGBE::Initialize(int argc, char* argv[])
{
	bool res = false;

	res = loadDefaultSettings();
	LOG_CRITICAL(res == false, return false, "Failed to load default settings");

	res = loadArguments(argc, argv);
	LOG_CRITICAL(res == false, return false, "Failed to load arguments");

	res = initializeSDL();
	LOG_CRITICAL(res == false, return false, "Failed to initialize SDL");

	// assert that the user inserted a rom file name
	LOG_ERROR(s_ROMFileName == "", return false, "Cannot initialize without a ROM file name");

	// initialize the interpreter
	m_Interpreter = new Interpreter();
	LOG_CRITICAL(m_Interpreter == nullptr , return false, "Failed to allocate memory for the interpreter");
	res = m_Interpreter->Initialize(s_ROMFileName);
	LOG_CRITICAL(res == false, return false, "Failed to initialize the interpreter");
	LOG_INFO(true, NOP, "SGBE initialized successfully." << endl);

	return true;
}

void SGBE::Run()
{
	if (m_Interpreter->IsCartridgeLoadedSuccessfully())
	{
		// main loop here
		m_Interpreter->Run();

		//SDL_SetRenderDrawColor(m_Renderer, 128, 70, 150, 255);
		////SDL_RenderDrawPoint(m_Renderer, GAMEBOY_SCREEN_WIDTH / 2, GAMEBOY_SCREEN_HEIGHT / 2); //Renders on middle of screen.
		//
		//for (int i = 0; i < GAMEBOY_SCREEN_WIDTH; i++)
		//{
		//	SDL_RenderDrawPoint(m_Renderer, i, i);
		//}
		//SDL_RenderPresent(m_Renderer);
		//
		//// todo remove this
		SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
		SDL_RenderClear(m_Renderer);
		SDL_RenderPresent(m_Renderer);
		SDL_Delay(5000);
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

bool SGBE::initializeSDL()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	m_Window = SDL_CreateWindow("SGBE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAMEBOY_SCREEN_WIDTH, GAMEBOY_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	LOG_CRITICAL(m_Window == NULL, return false, "Failed to create SDL window");

	m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
	LOG_CRITICAL(m_Window == NULL, return false, "Failed to create SDL renderer");

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

void SGBE::SDLDrawWrapper(byte i_R, byte i_G, byte i_B, uint32_t i_WidthPosition, uint32_t i_HeightPosition)
{
	SDL_SetRenderDrawColor(m_Renderer, i_R, i_G, i_B, 255);
	SDL_RenderDrawPoint(m_Renderer, i_WidthPosition, i_HeightPosition);
	SDL_RenderPresent(m_Renderer);
}
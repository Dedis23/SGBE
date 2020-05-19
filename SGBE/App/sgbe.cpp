#include "sgbe.h"

string SGBE::s_ROMFileName = "";

SGBE::SGBE(int argc, char* argv[]) : m_Window(nullptr), m_Renderer(nullptr), m_GBInterpreter(nullptr)
{
	bool res;

	res = loadDefaultSettings();
	LOG_CRITICAL(res == false, throw exception(), "Failed to load default settings");

	res = loadArguments(argc, argv);
	LOG_CRITICAL(res == false, throw exception(), "Failed to load arguments");

	res = initializeSDL();
	LOG_CRITICAL(res == false, throw exception(), "Failed to initialize SDL");

	res = initializeInterpreter();
	LOG_CRITICAL(res == false, throw exception(), "Failed to initialize the interpreter");
}

SGBE::~SGBE()
{
	delete m_GBInterpreter;
	m_GBInterpreter = nullptr;
}

void SGBE::Run()
{
	SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255); // todo remove this
	SDL_RenderClear(m_Renderer);
	SDL_RenderPresent(m_Renderer);
	SDL_Delay(3000);
}

bool SGBE::loadDefaultSettings()
{
	// set default logger settings
	LOGGER_SET_LOG_LEVEL(Logger::LogLevel::Critical);
	LOGGER_SET_LOG_TYPE(Logger::LogType::Console);

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

bool SGBE::initializeInterpreter()
{
	// assert that the user inserted a rom file name
	LOG_ERROR(s_ROMFileName == "", return false, "Cannot initialize without a ROM file name");

	try
	{
		m_GBInterpreter = new GBInterpreter(s_ROMFileName);
	}
	catch (const std::exception&)
	{
		return false;
	}
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
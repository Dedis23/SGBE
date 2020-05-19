#include "sgbe.h"

SGBE::SGBE(int argc, char* argv[]) : m_Window(nullptr), m_Renderer(nullptr)
{
	bool res;

	res = loadDefaultSettings();
	LOG_CRITICAL(res == false, throw exception(), "Failed to load default settings");

	res = loadArguments(argc, argv);
	LOG_CRITICAL(res == false, throw exception(), "Failed to load arguments");

	res = initializeSDL();
	LOG_CRITICAL(res == false, throw exception(), "Failed to initialize SDL");
}

void SGBE::Run()
{
	SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
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

	cli.AddOption("game", &loadRom);
	cli.LoadArgs(argc, argv);
	
	// CHECK THAT MAYBE THERE IS A BUG WITH CLI IS THAT WE CAN ADD MULTIPLE GAMES BUT IT WAS REGISTERED AS A SINGLE ARG (LIKE NOW)
	// MAYBE THIS IS NOW WORKING ACTUALLY

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

void SGBE::loadRom(const string& i_FileName)
{
	// TODO
	cout << "loaded rom: " << i_FileName << endl;
}
#include "sgbe.h"

SGBE::SGBE() : m_Window(nullptr), m_Renderer(nullptr)
{
	initialize();
}

void SGBE::Run()
{
	SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
	SDL_RenderClear(m_Renderer);
	SDL_RenderPresent(m_Renderer);
	SDL_Delay(3000);
}

void SGBE::initialize()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	m_Window = SDL_CreateWindow("SGBE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAMEBOY_SCREEN_WIDTH, GAMEBOY_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
}

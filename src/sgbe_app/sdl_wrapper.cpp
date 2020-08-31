#include "sdl_wrapper.h"

SDLWrapper::SDLWrapper() : m_WindowTitle(""), m_WindowWidth(0), m_WindowHeight(0), m_Window(nullptr), m_Renderer(nullptr), m_Texture(nullptr) {}

SDLWrapper::~SDLWrapper()
{
	SDL_DestroyRenderer(m_Renderer);
	SDL_DestroyWindow(m_Window);
	SDL_DestroyTexture(m_Texture);
	SDL_Quit();
}

bool SDLWrapper::Initialize(const string& i_WindowTitle, int i_WindowWidth, int i_WindowHeight)
{
	m_WindowTitle = i_WindowTitle;
	m_WindowWidth = i_WindowWidth;
	m_WindowHeight = i_WindowHeight;

	SDL_Init(SDL_INIT_EVERYTHING);

	m_Window = SDL_CreateWindow(m_WindowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_WindowWidth * 4, m_WindowHeight * 4, SDL_WINDOW_SHOWN);
	LOG_CRITICAL(m_Window == NULL, return false, "Failed to create SDL window");

	m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
	LOG_CRITICAL(m_Window == NULL, return false, "Failed to create SDL renderer");

	SDL_RenderSetLogicalSize(m_Renderer, m_WindowWidth, m_WindowHeight);

	m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, m_WindowWidth, m_WindowHeight);
	LOG_CRITICAL(m_Texture == NULL, return false, "Failed to create SDL texture");

	return true;
}

void SDLWrapper::RenderScreen(const Pixel* i_FrameBuffer)
{
	int res = -1;
	res = SDL_UpdateTexture(m_Texture, NULL, i_FrameBuffer, m_WindowWidth * sizeof(uint8_t) * 3);
	LOG_ERROR(res != 0, return, "SDL Failed to update texture");

	res = SDL_RenderClear(m_Renderer);
	LOG_ERROR(res != 0, return, "SDL Failed to render clear");

	res = SDL_RenderCopy(m_Renderer, m_Texture, NULL, NULL);
	LOG_ERROR(res != 0, return, "SDL Failed to render copy");

	SDL_RenderPresent(m_Renderer);
}
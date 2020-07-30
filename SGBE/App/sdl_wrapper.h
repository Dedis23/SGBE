/****************************************************************
 *			Created by: Dedi Sidi, 2020 					    *
 *															    *
 *			A basic SDL cpp wrapper with the minimum            *
 *          required features for the gameboy emulator          *
 ****************************************************************/

#pragma once
#include "SDL.h"
#include <string>
#include "../Generic/logger.h"

using namespace std;

struct Pixel;

class SDLWrapper
{
public:
    SDLWrapper();
    virtual ~SDLWrapper();
    SDLWrapper(const SDLWrapper&) = delete;
    SDLWrapper& operator=(const SDLWrapper&) = delete;

public:
    bool Initialize(const string& i_WindowTitle, int i_WindowWidth, int i_WindowHeight);
    void RenderScreen(Pixel* i_FrameBuffer);
    void Test() {}

private:
    string m_WindowTitle;
    int m_WindowWidth, m_WindowHeight;
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;
    SDL_Texture* m_Texture;
};
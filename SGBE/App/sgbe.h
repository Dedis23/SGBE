/****************************************************************
 *			Created by: Dedi Sidi, 2020 					    *
 *															    *
 *			The main application class						    *
 *          the application is based on the SDL framework       *
 ****************************************************************/

#pragma once
#include "SDL.h"
#include "../Generic/cli.h"
#include "../GB/gameboy.h"

class SGBE
{
public:
    SGBE();
    virtual ~SGBE();
    SGBE(const SGBE&) = delete;
    SGBE& operator=(const SGBE&) = delete;

    bool Initialize(int argc, char* argv[]);
    void Run();

private:
    bool loadDefaultSettings();
    bool loadArguments(int argc, char* argv[]);
    bool loadROM(const string& i_RomFilePath);
    bool initializeSDL();


private: // CLI Options
    static void cliRomOption(const string& i_RomFileName);
    static void cliSilentOption();
    static void cliLogFileNameOption(const string& i_LogFileName);

private: // SDL Wrappers
    void SDLRenderWrapper(Pixel i_FrameBuffer[]);

private:
    static string s_ROMFileName;
    vector<byte> m_ROMData;
    Gameboy* m_Gameboy;

    // SDL releated
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;
    SDL_Texture* m_Texture;
};
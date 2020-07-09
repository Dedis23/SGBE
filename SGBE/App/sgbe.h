/****************************************************************
 *			Created by: Dedi Sidi, 2020 					    *
 *															    *
 *			The main application class						    *
 *          the application is based on the SDL framework       *
 ****************************************************************/

#include "SDL.h"
#include "../Generic/cli.h"
#include "../GB/interpreter.h"

#ifndef __SGBE_H
#define __SGBE_H

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
    bool initializeSDL();


private:    // CLI Options
    static void cliRomOption(const string& i_RomFileName);
    static void cliSilentOption();
    static void cliLogFileNameOption(const string& i_LogFileName);

private: // SDL Wrappers
    void SDLDrawWrapper(byte i_R, byte i_G, byte i_B, uint32_t i_WidthPosition, uint32_t i_HeightPosition);

private:
    static string s_ROMFileName;
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;
    Interpreter* m_Interpreter;
};

#endif
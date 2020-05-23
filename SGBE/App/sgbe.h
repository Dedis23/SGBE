/****************************************************************
 *			Copyright (C) 2020 Dedi Sidi					    *
 *															    *
 *			The main application class						    *
 *          the application is based on the SDL framework       *
 ****************************************************************/

#ifndef __SGBE_H
#define __SGBE_H

#include "SDL.h"
#include "../Generic/cli.h"
#include "../GB/gb_utility.h"
#include "../GB/gb_interpreter.h"

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

    // CLI Options
private:
    static void cliRomOption(const string& i_RomFileName);
    static void cliSilentOption();
    static void cliLogFileNameOption(const string& i_LogFileName);
    static void cliDebugOption();

private:
    static string s_ROMFileName;
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;
    GBInterpreter* m_GBInterpreter;
};

#endif
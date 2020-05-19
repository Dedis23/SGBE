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

class SGBE
{
public:
    SGBE(int argc, char* argv[]);
	virtual ~SGBE() = default;
	SGBE(const SGBE&) = delete;
	SGBE& operator=(const SGBE&) = delete;

    void Run();

private:
    bool loadDefaultSettings();
    bool loadArguments(int argc, char* argv[]);
    bool initializeSDL();
    static void loadRom(const string& i_FileName);

private:
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;
};

#endif
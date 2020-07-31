/****************************************************************
 *			Created by: Dedi Sidi, 2020 					    *
 *															    *
 *			The main application class						    *
 *          the application is based on the SDL framework       *
 ****************************************************************/

#pragma once
#include "sdl_wrapper.h"
#include "../Generic/cli.h"
#include "../GB/gameboy.h"
#include <chrono>

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


private: // CLI Options
    static void cliRomOption(const string& i_RomFileName);
    static void cliSilentOption();
    static void cliLogFileNameOption(const string& i_LogFileName);

private:
    static string s_ROMFileName;
    vector<byte> m_ROMData;
    SDLWrapper* m_SDLWrapper;
    Gameboy* m_Gameboy;
};
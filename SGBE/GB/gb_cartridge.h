/********************************************************************
 *			Copyright (C) 2020 Dedi Sidi			                *
 *														            *
 *			Cartridge class that is specific for the gameboy        *
 ********************************************************************/

#ifndef __GB_CARTRIDGE_
#define __GB_CARTRIDGE_

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include "../Generic/logger.h"

using namespace std;

class GBCartridge
{
public:
    GBCartridge(const string& i_RomFileName);
	virtual ~GBCartridge() = default;

private:
    bool loadROM(const string& i_RomFilePath);

protected:
	vector<uint8_t> m_ROMData;
};

#endif
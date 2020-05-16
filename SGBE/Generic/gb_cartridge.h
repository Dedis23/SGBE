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

using namespace std;

class GBCartridge
{
public:
    GBCartridge(const string& i_FilePath);
	virtual ~GBCartridge() = default;

protected:
	vector<uint8_t> mData;
};

#endif
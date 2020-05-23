/********************************************************************
 *			Copyright (C) 2020 Dedi Sidi			                *
 *														            *
 *			A class that holds metadata about the cartridge         *
 ********************************************************************/

#ifndef __GB_METADATA_CARTRIDGE_
#define __GB_METADATA_CARTRIDGE_

#include "../gb_utility.h"
#include <vector>
#include <string>

/* Cartridge metadata is held within addresses: 0x0100 - 0x014F  */
/* Most of the info was taken from http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-Memory-Banking */

class GBCartridgeMetaData
{
public:
    enum class CartridgeType
    {
        NoMBC,
        UnknownCartridgeType,
    };

    enum class DestinationType
    {
        Japanese,
        NonJapanese, // rest of the world
        UnknownDestinationType,
    };

public:
    GBCartridgeMetaData(const vector<byte>& i_ROMData);
	virtual ~GBCartridgeMetaData() = default;

    std::string Title;
    CartridgeType CartridgeType;
    DestinationType DestinationType;

private:
    void extractTitle();

private:
    const vector<byte>& m_ROMData;
};

#endif
/************************************************
 *          Created by: Dedi Sidi, 2020         *
 *                                              *
 *          cartridge factory class             *
 ************************************************/

#pragma once
#include "cartridge.h"

class CartridgeFactory
{
public:
    CartridgeFactory() = delete;
	virtual ~CartridgeFactory() = default;
    CartridgeFactory(const CartridgeFactory&) = delete;
	CartridgeFactory& operator=(const CartridgeFactory&) = delete;

    static Cartridge* CreateCartridge(vector<byte>& i_ROMData, const CartridgeHeader& i_CartridgeHeader);
};
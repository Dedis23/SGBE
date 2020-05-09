#ifndef __CARTRIDGE_
#define __CARTRIDGE_

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Cartridge
{
public:
	Cartridge(const string& i_FilePath);
	virtual ~Cartridge() = default;

protected:
	vector<uint8_t> mData;
};

#endif
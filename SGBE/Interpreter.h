#ifndef __INTERPRETER_H
#define __INTERPRETER_H

#include "GB/CPU/gb_cpu.h"

class Interpreter
{
public:
	Interpreter() = default;
	~Interpreter() = default;
	void Run();

private:
	GBCPU m_CPU;
};

#endif
#ifndef __INTERPRETER_H
#define __INTERPRETER_H

#include "GB/CPU/gb_cpu.h"

class Interpreter
{
public:
	Interpreter() = default;
	virtual ~Interpreter() = default;
	Interpreter(const Interpreter&) = delete;
	Interpreter& operator=(const Interpreter&) = delete;

	void Run();

private:
	GBCPU m_CPU;
};

#endif
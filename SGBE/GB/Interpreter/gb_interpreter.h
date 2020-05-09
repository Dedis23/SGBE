#ifndef __GB_INTERPRETER_H
#define __GB_INTERPRETER_H

#include "../CPU/gb_cpu.h"

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
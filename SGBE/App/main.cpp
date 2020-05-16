#include "sgbe.h"

int main(int argc, char* argv[])
{
	LOGGER_SET_LOG_TYPE(Logger::Log_Type::Console);

	//SGBE sgbe;
	//sgbe.Run();

	LOG_INFO(true, exit(EXIT_FAILURE), "HELLO WORLD!");

	return 0;
}
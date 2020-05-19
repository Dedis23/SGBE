#include "sgbe.h"

int main(int argc, char* argv[])
{
	SGBE* sgbe;

	try
	{
		sgbe = new SGBE(argc, argv);
	}
	catch (const std::exception&)
	{
		LOG_CRITICAL(true, exit(EXIT_FAILURE), "Failed to initialize SGBE");
	}

	sgbe->Run();
	delete sgbe;

	return EXIT_SUCCESS;
}
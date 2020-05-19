#include "sgbe.h"

int main(int argc, char* argv[])
{
	SGBE* sgbe = nullptr;
	int exitStatus = EXIT_SUCCESS;

	try
	{
		sgbe = new SGBE(argc, argv);
	}
	catch (const std::exception&)
	{
		LOG_CRITICAL(true, exitStatus = EXIT_FAILURE, "Failed to initialize SGBE");
	}

	sgbe->Run();
	delete sgbe;
	Logger::ResetInstance();

	return exitStatus;
}
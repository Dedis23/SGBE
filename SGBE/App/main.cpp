#include "sgbe.h"

int main(int argc, char* argv[])
{
	bool res = false;
	SGBE* sgbe = nullptr;
	int exitStatus = EXIT_SUCCESS;

	sgbe = new SGBE();
	LOG_CRITICAL(sgbe == nullptr, return EXIT_FAILURE, "Failed to allocate memory for SGBE");
	
	res = sgbe->Initialize(argc, argv);
	LOG_CRITICAL(res == false, delete sgbe; return EXIT_FAILURE, "Failed to initialize SGBE");

	sgbe->Run();

	delete sgbe;
	Logger::ResetInstance();

	return exitStatus;
}
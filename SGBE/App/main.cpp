#include "sgbe.h"

int main(int argc, char* argv[])
{
	ByteRegister a(257);
	cout << a.GetValue() << endl;

	a.SetValue(259);
	cout << a.GetValue() << endl;

	ByteRegister testReg(1);
	testReg.SetBit(7, true);
	
	for (int i = 7; i >= 0; i--)
	{
		std::cout << testReg.GetBit(i);
	}
	std::cout << std::endl;
	
	testReg.Clear();

	for (int i = 7; i >= 0; i--)
	{
		std::cout << testReg.GetBit(i);
	}
	std::cout << std::endl;

	testReg.SetValue(255);

	for (int i = 7; i >= 0; i--)
	{
		std::cout << testReg.GetBit(i);
	}
	std::cout << std::endl;

	testReg.Increment();
	for (int i = 7; i >= 0; i--)
	{
		std::cout << testReg.GetBit(i);
	}
	std::cout << std::endl;

	testReg.Decrement();
	for (int i = 7; i >= 0; i--)
	{
		std::cout << testReg.GetBit(i);
	}
	std::cout << std::endl;

	testReg.Decrement();
	for (int i = 7; i >= 0; i--)
	{
		std::cout << testReg.GetBit(i);
	}
	std::cout << std::endl;

	sbyte sb = static_cast<sbyte>(testReg.GetValue());
	cout << (int)sb << endl;

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
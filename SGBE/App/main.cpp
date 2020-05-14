#include "SDL.h"
#include "../Generic/registers.h"
#include "../GB/CPU/gb_registers.h"
#include "../Generic/logger.h"
#include <thread>

#define NUM_OF_TESTS 1000

int test1(int tnum)
{
	for (int i = 0; i < NUM_OF_TESTS; i++)
	{

		stringstream strs;
		strs << "A message number: " << i + 1 << " from thread number: " << tnum;
		//LOGGER_SET_LOG_LEVEL(Logger::Log_Level::Critical);
		LOG_ERROR(strs.str());
	}
	return 0;
}

int test2(int tnum)
{
	for (int i = 0; i < NUM_OF_TESTS; i++)
	{

		stringstream strs;
		strs << "A message number: " << i + 1 << " from thread number: " << tnum;
		//LOGGER_SET_LOG_LEVEL(Logger::Log_Level::Critical);
		LOG_INFO(strs.str());
	}
	return 0;
}

int main(int argc, char* argv[])
{
	//SDL_Init(SDL_INIT_EVERYTHING);
	//SDL_Window* window = SDL_CreateWindow("Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 400, SDL_WINDOW_SHOWN);
	//SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	//
	//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	//
	//SDL_RenderClear(renderer);
	//SDL_RenderPresent(renderer);
	//
	//SDL_Delay(3000);

	//ByteRegister testReg(1);
	//testReg.SetBit(8, true);
	//
	//for (int i = 8; i > 0; i--)
	//{
	//	std::cout << testReg.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//testReg.Clear();
	//
	//for (int i = 8; i > 0; i--)
	//{
	//	std::cout << testReg.GetBit(i);
	//}
	//std::cout << std::endl;

	//FlagRegister flagRegisterTest(0);
	//
	//for (int i = 8; i > 0; i--)
	//{
	//	std::cout << flagRegisterTest.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//flagRegisterTest.SetZFlag(true);
	//flagRegisterTest.SetNFlag(true);
	//flagRegisterTest.SetHFlag(true);
	//flagRegisterTest.SetCFlag(true);
	//
	//for (int i = 8; i > 0; i--)
	//{
	//	std::cout << flagRegisterTest.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//flagRegisterTest.SetZFlag(false);
	//
	//for (int i = 8; i > 0; i--)
	//{
	//	std::cout << flagRegisterTest.GetBit(i);
	//}
	//std::cout << std::endl;

	//WordRegister a(60000);
	//std::cout << a.GetValue() << std::endl;
	//
	//for (int i = 16; i > 0; i--)
	//{
	//	std::cout << a.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//a.SetBit(1, true);
	//a.SetBit(3, true);
	//
	//for (int i = 16; i > 0; i--)
	//{
	//	std::cout << a.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//std::cout << "Low byte is: " << a.GetLowByte() << std::endl;
	//for (int i = 8; i > 0; i--)
	//{
	//	std::cout << a.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//std::cout << "High byte is: " << a.GetHighByte() << std::endl;
	//for (int i = 16; i > 8; i--)
	//{
	//	std::cout << a.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//std::cout << "Changing low byte:" << std::endl;
	//uint8_t test = 0;
	//a.SetLowByte(test);
	//for (int i = 8; i > 0; i--)
	//{
	//	std::cout << a.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//uint8_t test2 = 188;
	//a.SetHighByte(test2);
	//std::cout << "Changing high byte:" << std::endl;
	//for (int i = 16; i > 8; i--)
	//{
	//	std::cout << a.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//ByteRegister low(145);
	//ByteRegister high(222);
	//
	//Pair8BRegisters pair(high, low);
	//
	//pair.SetValue(62000);
	//
	//uint16_t combinedValue = pair.GetValue();
	//a.SetValue(combinedValue);
	//
	//for (int i = 16; i > 0; i--)
	//{
	//	std::cout << a.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//std::cout << "high byte is:" << std::endl;
	//for (int i = 8; i > 0; i--)
	//{
	//	std::cout << high.GetBit(i);
	//}
	//std::cout << std::endl;
	//
	//std::cout << "low byte is:" << std::endl;
	//for (int i = 8; i > 0; i--)
	//{
	//	std::cout << low.GetBit(i);
	//}
	//std::cout << std::endl;

	//std::cout << a.GetValue() << std::endl;
	//
	//a.Decrement();
	//
	//std::cout << a.GetValue() << std::endl;
	//
	//a.Clear();
	//
	//std::cout << a.GetValue() << std::endl;
	//
	//a.Decrement();
	//
	//std::cout << a.GetValue() << std::endl;

LOGGER_SET_FILE_NAME("thread tests.log");

//thread t1(test1, 1);
//
//thread t2(test2, 2);
//
//t1.join();
//t2.join();


string(__FILE__).substr(string(__FILE__).find_last_of(R"(\)") + 1);


	//LOG_INFO("DEDI SIDI EFG");
	//LOG_INFO_TO_FILE("HIJKLMNOP");
	//
	//LOGGER_SET_FILE_NAME("dedi_sidi.log");
	//
	//LOG_INFO_TO_FILE("QRSTUVW");
	//LOG_INFO_TO_FILE("XYZ");

	//cout << Logger::GetInstance()->getCurrentTime() << endl;

	return 0;
}
#include "SDL.h"
#include "../Generic/registers.h"
#include "../GB/CPU/gb_registers.h"

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

	WordRegister a(60000);
	std::cout << a.GetValue() << std::endl;
	
	for (int i = 16; i > 0; i--)
	{
		std::cout << a.GetBit(i);
	}
	std::cout << std::endl;
	
	a.SetBit(1, true);
	a.SetBit(3, true);
	
	for (int i = 16; i > 0; i--)
	{
		std::cout << a.GetBit(i);
	}
	std::cout << std::endl;

	std::cout << "Low byte is: " << a.GetLowByte() << std::endl;
	for (int i = 8; i > 0; i--)
	{
		std::cout << a.GetBit(i);
	}
	std::cout << std::endl;

	std::cout << "High byte is: " << a.GetHighByte() << std::endl;
	for (int i = 16; i > 8; i--)
	{
		std::cout << a.GetBit(i);
	}
	std::cout << std::endl;

	std::cout << "Changing low byte:" << std::endl;
	uint8_t test = 0;
	a.SetLowByte(test);
	for (int i = 8; i > 0; i--)
	{
		std::cout << a.GetBit(i);
	}
	std::cout << std::endl;

	uint8_t test2 = 188;
	a.SetHighByte(test2);
	std::cout << "Changing high byte:" << std::endl;
	for (int i = 16; i > 8; i--)
	{
		std::cout << a.GetBit(i);
	}
	std::cout << std::endl;

	ByteRegister low(145);
	ByteRegister high(222);

	Pair8BRegisters pair(high, low);

	pair.SetValue(62000);

	uint16_t combinedValue = pair.GetValue();
	a.SetValue(combinedValue);

	for (int i = 16; i > 0; i--)
	{
		std::cout << a.GetBit(i);
	}
	std::cout << std::endl;

	std::cout << "high byte is:" << std::endl;
	for (int i = 8; i > 0; i--)
	{
		std::cout << high.GetBit(i);
	}
	std::cout << std::endl;

	std::cout << "low byte is:" << std::endl;
	for (int i = 8; i > 0; i--)
	{
		std::cout << low.GetBit(i);
	}
	std::cout << std::endl;

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

	return 0;
}
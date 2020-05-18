#include "sgbe.h"
#include "../Generic/cli.h"

void file(const vector<string>& i_Args) 
{
	cout << "Game files are: ";
	for (int i = 0; i < i_Args.size(); i++)
	{
		cout << i_Args[i] << " ";
	}
	cout << endl;
}

void silent(const vector<string>& i_Args)
{
	cout << "Silent mode is on" << endl;
}

void debug(const vector<string>& i_Args)
{
	cout << "Debug mode is on" << endl;
}

int main(int argc, char* argv[])
{
	//LOGGER_SET_LOG_TYPE(Logger::Log_Type::Console);

	//SGBE sgbe;
	//sgbe.Run();

	//LOG_INFO(true, exit(EXIT_FAILURE), "HELLO WORLD!");

	CLI cli;
	cli.AddOption({ "file" }, &file);
	cli.AddOption({ "silent" }, &silent);
	cli.AddOption({ "debug" }, &debug);

	cli.LoadArgs(argc, argv);

	return 0;
}
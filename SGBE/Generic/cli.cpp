#include "cli.h"

CLI::CLI(const string& i_OptionDelimiter) : m_OptionDelimiter(i_OptionDelimiter) {}

void CLI::AddOption(const string& i_Option, OptionFunction i_Function)
{
	m_OptionsMap.insert(std::make_pair(i_Option, i_Function));
}

void CLI::AddOption(const string& i_Option, OptionFunctionMultipleArgs i_FunctionWithMultipleArgs)
{
	m_MultipleArgsOptionsMap.insert(std::make_pair(i_Option, i_FunctionWithMultipleArgs));
}

void CLI::LoadArgs(int argc, char* argv[])
{
	vector<string> options;
	vector<vector<string>> optionsArguments;
	string currStr;
	int currOption = -1;
	size_t pos = 0;

	if (argc < 2)
	{
		return;
	}

	// parse the options and the arguments
	for (int i = 1; i < argc; i++)
	{
		currStr = argv[i];
		if (currStr.find(m_OptionDelimiter) != string::npos) // option
		{
			currOption++;
			optionsArguments.push_back({});
			currStr = currStr.substr(currStr.find(m_OptionDelimiter) + m_OptionDelimiter.size());
			options.push_back(currStr);
		}
		else // argument of option
		{
			if (currOption > -1)
			{
				optionsArguments[currOption].push_back(currStr);
			}
		}
	}
	
	// invoke every function of every option
	unordered_map<string, OptionFunction>::const_iterator itrSingleArg;
	unordered_map<string, OptionFunctionMultipleArgs>::const_iterator itrMultipleArgs;
	for (int i = 0; i < options.size(); i++)
	{
		itrSingleArg = m_OptionsMap.find(options[i]);
		if (itrSingleArg != m_OptionsMap.end())
		{
			itrSingleArg->second(optionsArguments[i][0]);
			continue;
		}
		itrMultipleArgs = m_MultipleArgsOptionsMap.find(options[i]);
		if (itrMultipleArgs != m_MultipleArgsOptionsMap.end())
		{
			itrMultipleArgs->second(optionsArguments[i]);
		}
	}
}
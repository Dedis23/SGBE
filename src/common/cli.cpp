#include "cli.h"

CLI::CLI(const string& i_OptionDelimiter) : m_OptionDelimiter(i_OptionDelimiter) {}


void CLI::AddNoOptionNoArg(NoOptionFunctionSingleArg i_Function)
{
	m_NoOptionFunctionSingleArg = i_Function;
}

void CLI::AddOption(const string& i_Option, OptionFunctionNoArg i_Function)
{
	m_NoArgsOptionsMap.insert(std::make_pair(i_Option, i_Function));
}

void CLI::AddOption(const string& i_Option, OptionFunctionSingleArg i_Function)
{
	m_SingleArgOptionsMap.insert(std::make_pair(i_Option, i_Function));
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
	unordered_map<string, OptionFunctionNoArg>::const_iterator itrNoArgMap;
	unordered_map<string, OptionFunctionSingleArg>::const_iterator itrSingleArgMap;
	unordered_map<string, OptionFunctionMultipleArgs>::const_iterator itrMultipleArgsMap;
	for (size_t i = 0; i < options.size(); i++)
	{
		itrNoArgMap = m_NoArgsOptionsMap.find(options[i]);
		if (itrNoArgMap != m_NoArgsOptionsMap.end())
		{
			itrNoArgMap->second();
			continue;
		}
		itrSingleArgMap = m_SingleArgOptionsMap.find(options[i]);
		if (itrSingleArgMap != m_SingleArgOptionsMap.end())
		{
			if (optionsArguments[i].size() != 0)
			{
				itrSingleArgMap->second(optionsArguments[i][0]);
			}
			continue;
		}
		itrMultipleArgsMap = m_MultipleArgsOptionsMap.find(options[i]);
		if (itrMultipleArgsMap != m_MultipleArgsOptionsMap.end())
		{
			itrMultipleArgsMap->second(optionsArguments[i]);
		}
	}
}
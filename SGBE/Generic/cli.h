/********************************************************************************
 *			Copyright (C) 2020 Dedi Sidi	                                    *
 *											                                    *
 *			A very basic CLI parsing system                                     *
 ********************************************************************************/

#ifndef __CLI_H
#define __CLI_H

#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

typedef void (*OptionFunction)(const string&);
typedef void (*OptionFunctionMultipleArgs)(const vector<string>&);

class CLI
{
public:
    CLI(const string& i_OptionDelimiter = "--");
	virtual ~CLI() = default;
    CLI(const CLI&) = delete;
    CLI& operator=(const CLI&) = delete;

    void AddOption(const string& i_Option, OptionFunction i_Function);
    void AddOption(const string& i_Option, OptionFunctionMultipleArgs i_FunctionWithMultipleArgs);
    void LoadArgs(int argc, char* argv[]);

private:
    unordered_map<string, OptionFunction> m_OptionsMap;
    unordered_map<string, OptionFunctionMultipleArgs> m_MultipleArgsOptionsMap;
    string m_OptionDelimiter;
};

#endif
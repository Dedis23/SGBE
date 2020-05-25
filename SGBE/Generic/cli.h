/********************************************************************************
 *			Copyright (C) 2020 Dedi Sidi	                                    *
 *											                                    *
 *			A very basic CLI parsing system                                     *
 ********************************************************************************/

#include <sstream>
#include <vector>
#include <unordered_map>

#ifndef __CLI_H
#define __CLI_H

using namespace std;

typedef void (*OptionFunctionNoArg)();
typedef void (*OptionFunctionSingleArg)(const string&);
typedef void (*OptionFunctionMultipleArgs)(const vector<string>&);

class CLI
{
public:
    CLI(const string& i_OptionDelimiter = "--");
    virtual ~CLI() = default;
    CLI(const CLI&) = delete;
    CLI& operator=(const CLI&) = delete;

    void AddOption(const string& i_Option, OptionFunctionNoArg i_Function);
    void AddOption(const string& i_Option, OptionFunctionSingleArg i_Function);
    void AddOption(const string& i_Option, OptionFunctionMultipleArgs i_FunctionWithMultipleArgs);
    void LoadArgs(int argc, char* argv[]);

private:
    unordered_map<string, OptionFunctionNoArg> m_NoArgsOptionsMap;
    unordered_map<string, OptionFunctionSingleArg> m_SingleArgOptionsMap;
    unordered_map<string, OptionFunctionMultipleArgs> m_MultipleArgsOptionsMap;
    string m_OptionDelimiter;
};

#endif
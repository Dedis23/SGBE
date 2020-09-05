/****************************************************
 *          Created by: Dedi Sidi, 2020             *
 *                                                  *
 *          A basic CLI parsing system              *
 ****************************************************/

#pragma once
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

typedef void (*NoOptionFunctionSingleArg)(const string&);
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

    /* this will be invoked from the argument string that does not have option delimeter */
    void AddNoOptionNoArg(NoOptionFunctionSingleArg i_Function);

    /* these function will be invoked once the option will be parsesd */
    /* there is a type for no args, single args and multiple args */
    void AddOption(const string& i_Option, OptionFunctionNoArg i_Function);
    void AddOption(const string& i_Option, OptionFunctionSingleArg i_Function);
    void AddOption(const string& i_Option, OptionFunctionMultipleArgs i_FunctionWithMultipleArgs);

    /* actual parsing happens here */
    void LoadArgs(int argc, char* argv[]);

private:
    unordered_map<string, OptionFunctionNoArg> m_NoArgsOptionsMap;
    unordered_map<string, OptionFunctionSingleArg> m_SingleArgOptionsMap;
    unordered_map<string, OptionFunctionMultipleArgs> m_MultipleArgsOptionsMap;
    NoOptionFunctionSingleArg m_NoOptionFunctionSingleArg;
    string m_OptionDelimiter;
};
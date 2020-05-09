#ifndef __LOGGER_H
#define __LOGGER_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <ctime>

using namespace std;

/****************************************
 * Generic thread-safe Logger singleton *
 ****************************************/

/* Direct interface using macros */

/* regular interface, log based on current type */

/* explicitly log into file */
#define LOG_INFO_TO_FILE(x) Logger::GetInstance()->InfoToFile(x)
#define LOGGER_SET_FILE_NAME(x) Logger::GetInstance()->SetFileName(x)

class Logger
{
public:
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	virtual ~Logger() = default;

	static Logger* GetInstance();
	static void SetFileName(const string& i_FileName);

	void Info(const string& i_Message);
	void Error(const string& i_Message);
	void Critical(const string& i_Message);

	void InfoToFile(const string& i_Message);
	void ErrorToFile(const string& i_Message);
	void CriticalToFile(const string& i_Message);

	enum class Log_Level
	{
		Disabled = 0,
		Info = 1,
		Error = 2,
		Critical = 4, // ALL LOGS
	};

	enum class Log_Type
	{
		Console = 0,
		File = 1,
	};

private:
	Logger();
	static void initInstance();
	static void initFileStream();
	void writeToFile(const string& i_Message);
	string getCurrentTime();

private:
	static Logger* s_Instance;
	static once_flag s_OnceFlag;
	std::mutex m_FileLock;
	static std::ofstream m_FileStream;
	static string m_FileName;
	Log_Level m_LogLevel = Log_Level::Critical;
	Log_Type m_LogType = Log_Type::File;
};

#endif
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
#define LOG_INFO(x) LOGGER_HEADLINE(x) Logger::GetInstance()->Info(ss.str())
#define LOG_ERROR(x) LOGGER_HEADLINE(x) Logger::GetInstance()->Error(ss.str())
#define LOG_CRITICAL(x) LOGGER_HEADLINE(x) Logger::GetInstance()->Critical(ss.str())

/* Direct interface to change logger options */
#define LOGGER_SET_FILE_NAME(x) Logger::GetInstance()->SetFileName(x)
#define LOGGER_SET_LOG_LEVEL(x) Logger::GetInstance()->SetLogLevel(x)
#define LOGGER_SET_LOG_TYPE(x) Logger::GetInstance()->SetLogType(x)

/* utility macros, not be used outside */
#define LOGGER_HEADLINE(x) __FILENAME__ stringstream ss; ss << f << ":" << __LINE__ << " - " << __func__ << "] --> " << x;
#define __FILENAME__ string f = __FILE__; f = f.substr(f.find_last_of(R"(\)") + 1);

class Logger
{
public:
	enum class Log_Level
	{
		Disabled = 0,
		Info = 1,
		Error = 2,
		Critical = 3, // ALL LOGS
	};

	enum class Log_Type
	{
		Console = 0,
		File = 1,
	};

public:
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	virtual ~Logger() = default;

	static Logger* GetInstance();
	static void SetFileName(const string& i_FileName);
	static void SetLogLevel(const Log_Level& i_LogLevel);
	static void SetLogType(const Log_Type& i_LogType);

	void Info(const string& i_Message);
	void Error(const string& i_Message);
	void Critical(const string& i_Message);

private:
	Logger();
	static void initInstance();
	static void initFileStream();
	string constructLog(const string& i_Headline, const string& i_Message);
	void outputLog(const string& i_Message);
	string getCurrentTime();

private:
	static Logger* s_Instance;
	static once_flag s_OnceFlag;
	static std::mutex s_FileLock;
	static std::ofstream m_FileStream;
	static string s_FileName;
	Log_Level m_LogLevel = Log_Level::Critical;
	Log_Type m_LogType = Log_Type::File;
	const string c_InfoHeadline = "[INFO]:";
	const string c_ErrorHeadline = "[ERROR]:";
	const string c_CriticalHeadline = "[CRITICAL]:";
};

#endif
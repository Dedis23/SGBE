/****************************************
 *	Generic thread-safe Logger class	*
 ****************************************/

#ifndef __LOGGER_H
#define __LOGGER_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <ctime>
#include <vector>

using namespace std;

/* Direct interface using macros */
#define LOG_INFO(message) LOG_METADATA(message) Logger::GetInstance()->Info(metaDataAndMsg.str())
#define LOG_ERROR(message) LOG_METADATA(message) Logger::GetInstance()->Error(metaDataAndMsg.str())
#define LOG_CRITICAL(message) LOG_METADATA(message) Logger::GetInstance()->Critical(metaDataAndMsg.str())

/* Direct interface to change logger options */
#define LOGGER_SET_FILE_NAME(fileName) Logger::GetInstance()->SetFileName(fileName)
#define LOGGER_SET_LOG_LEVEL(LogLevel) Logger::GetInstance()->SetLogLevel(LogLevel)
#define LOGGER_SET_LOG_TYPE(LogType) Logger::GetInstance()->SetLogType(LogType)

/* utility macros to get log metadata, not be used outside */
#define LOG_METADATA(message) stringstream metaDataAndMsg; metaDataAndMsg << __FILE__ << "\n" << __func__ << "\n" << __LINE__ << "\n" << message;

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
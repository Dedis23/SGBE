/********************************************************
 *			Copyright (C) 2020 Dedi Sidi				*
 *														*
 *			Generic thread-safe Logger class			*
 ********************************************************/

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

/****************************************************
 *			Direct interface using macros			*
 *			Use only these macros to write logs     *
 ****************************************************/

/* Direct interface to log in info level */
#define LOG_INFO(condition, action, message) \
	{	LOG_METADATA(message) \
		CHECK_CONDITION(condition, action, Logger::GetInstance()->Info(metaDataAndMsg.str())) \
	} static_assert(true, "")

/* Direct interface to log in error level */
#define LOG_ERROR(condition, action, message) \
	{	LOG_METADATA(message) \
		CHECK_CONDITION(condition, action, Logger::GetInstance()->Error(metaDataAndMsg.str())) \
	} static_assert(true, "")

/* Direct interface to log in critical level */
#define LOG_CRITICAL(condition, action, message) \
	{	LOG_METADATA(message) \
		CHECK_CONDITION(condition, action, Logger::GetInstance()->Critical(metaDataAndMsg.str())) \
	} static_assert(true, "")

// Note - the "static_assert(true, "")" at the end is to force semicolon upon usage

/**********************************************
 * Direct interface to change logger options  *
 **********************************************/

/* Direct interface to change the logger filename */
#define LOGGER_SET_FILE_NAME(fileName) Logger::GetInstance()->SetFileName(fileName)

/* Direct interface to set the logging level: INFO, ERROR, CRITICAL or DISABLED */
#define LOGGER_SET_LOG_LEVEL(LogLevel) Logger::GetInstance()->SetLogLevel(LogLevel)

/* Direct interface to set the logger output type: File or Console */
#define LOGGER_SET_LOG_TYPE(LogType) Logger::GetInstance()->SetLogType(LogType)

/* Direct interface to enable / disable the logger metadata in the output */
#define LOGGER_SET_LOG_METADATA(_bool) Logger::GetInstance()->SetLogMetaData(_bool)

/**************************************************************
 * Utility macros to get log metadata, NOT TO BE USED OUTSDIE *
 **************************************************************/

#define LOG_METADATA(message) \
	stringstream metaDataAndMsg; \
	metaDataAndMsg << __FILE__ << "\n" << __func__ << "\n" << __LINE__ << "\n" << message;

#define CHECK_CONDITION(condition, action, loggerFunction) \
	if (condition) \
	{ \
		loggerFunction; \
		action; \
	} 

/***************************
 *		Logger class       *
 ***************************/

class Logger
{
public:
	enum class LogLevel
	{
		Disabled = 0,
		Critical = 1,
		Error = 2,
		Info = 3, // ALL LOGS
	};

	enum class LogType
	{
		Console = 0,
		File = 1,
	};

public:
	virtual ~Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	static Logger* GetInstance();
	static void ResetInstance();
	static void SetFileName(const string& i_FileName);
	static void SetLogLevel(const LogLevel& i_LogLevel);
	static void SetLogType(const LogType& i_LogType);
	static void SetLogMetaData(bool i_IsMetaDataOn);

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
	static bool s_IsMetaData;
	LogLevel m_LogLevel = LogLevel::Critical;
	LogType m_LogType = LogType::File;
	const string c_InfoHeadline = "[INFO]:";
	const string c_ErrorHeadline = "[ERROR]:";
	const string c_CriticalHeadline = "[CRITICAL]:";
};

#endif
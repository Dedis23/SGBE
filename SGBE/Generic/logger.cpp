#include "logger.h"

std::once_flag Logger::s_OnceFlag;
std::mutex Logger::s_FileLock;
Logger* Logger::s_Instance = nullptr;
std::ofstream Logger::m_FileStream;
string Logger::s_FileName = "Logger.log"; // default file name

Logger::Logger()
{
	// lock
	std::lock_guard<std::mutex> lock(s_FileLock);
}

Logger* Logger::GetInstance()
{
	if (s_Instance == nullptr)
	{
		std::call_once(s_OnceFlag, initInstance);
	}
	return s_Instance;
}

void Logger::SetFileName(const string& i_FileName)
{
	// lock
	std::lock_guard<std::mutex> lock(s_FileLock);

	/* if the stream is already open with a different name */
	if (m_FileStream.is_open())
	{
		m_FileStream.close();
	}
	// set the file name to the new name and reinit the file stream
	s_FileName = i_FileName;
}

void Logger::SetLogLevel(const Log_Level& i_LogLevel)
{
	std::lock_guard<std::mutex> lock(s_FileLock);
	Logger::GetInstance()->m_LogLevel = i_LogLevel;
}

void Logger::SetLogType(const Log_Type& i_LogType)
{
	std::lock_guard<std::mutex> lock(s_FileLock);
	Logger::GetInstance()->m_LogType = i_LogType;
}

void Logger::Info(const string& i_Message)
{
	if (m_LogLevel >= Logger::Log_Level::Info)
	{
		outputLog(constructLog(c_InfoHeadline, i_Message));
	}
}

void Logger::Error(const string& i_Message)
{
	if (m_LogLevel >= Logger::Log_Level::Error)
	{
		outputLog(constructLog(c_ErrorHeadline, i_Message));
	}
}

void Logger::Critical(const string& i_Message)
{
	if (m_LogLevel >= Logger::Log_Level::Critical)
	{
		outputLog(constructLog(c_CriticalHeadline, i_Message));
		exit(EXIT_FAILURE);
	}
}

void Logger::initInstance()
{
	s_Instance = new Logger();
}

void Logger::initFileStream()
{
	m_FileStream.open(s_FileName.c_str(), std::ios_base::app);
	if (!m_FileStream.is_open() || !m_FileStream.good())
	{
		throw "Error opening file!";
	}
}

string Logger::constructLog(const string& i_Headline, const string& i_Message)
{
	stringstream ss;
	ss << i_Headline << " [" << getCurrentTime() << " - " << i_Message;
	return ss.str();
}

void Logger::outputLog(const string& i_Message)
{
	// lock
	std::lock_guard<std::mutex> lock(s_FileLock);

	// output
	if (m_LogType == Logger::Log_Type::File)
	{
		// init filestream upon first  writing
		if (!m_FileStream.is_open())
		{
			initFileStream();
		}
		// write to file
		m_FileStream << i_Message << std::endl;
	}
	else
	{
		// write to standart output
		cout << i_Message << endl;
	}
}

string Logger::getCurrentTime()
{
	std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());;
	char buffer[20];
	tm t;
	localtime_s(&t, &now_time);
	strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &t);
	return buffer;
}
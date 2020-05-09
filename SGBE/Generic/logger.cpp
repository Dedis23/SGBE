#include "logger.h"

std::once_flag Logger::s_OnceFlag;
Logger* Logger::s_Instance = nullptr;
std::ofstream Logger::m_FileStream;
string Logger::m_FileName = "Logger.log"; // default file name

Logger::Logger()
{
	initFileStream();
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
	/* if the stream is already open with a different name */
	if (m_FileStream.is_open())
	{
		m_FileStream.close();
	}
	// set the file name to the new name and reinit the file stream
	m_FileName = i_FileName;
	initFileStream();

}

void Logger::Info(const string& i_Message)
{
	switch (m_LogType)
	{
	case Logger::Log_Type::Console:
		break;
	case Logger::Log_Type::File:
		InfoToFile(i_Message);
		break;
	default:
		break;
	}
}

void Logger::Error(const string& i_Message)
{

}

void Logger::Critical(const string& i_Message)
{

}

void Logger::InfoToFile(const string& i_Message)
{
	if (m_LogLevel >= Logger::Log_Level::Info)
	{
		string msg("[INFO]: ");
		msg.append(i_Message);
		writeToFile(msg);
	}
}

void Logger::ErrorToFile(const string& i_Message)
{
}

void Logger::CriticalToFile(const string& i_Message)
{
}

void Logger::initInstance()
{
	s_Instance = new Logger();
}

void Logger::initFileStream()
{
	m_FileStream.open(m_FileName.c_str(), std::ios_base::app);
	if (!m_FileStream.is_open() || !m_FileStream.good())
	{
		throw "Error opening file!";
	}
}

void Logger::writeToFile(const string& i_Message)
{
	std::lock_guard<std::mutex> lock(m_FileLock);
	m_FileStream << i_Message << std::endl;
}

string Logger::getCurrentTime()
{
	time_t time = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	cout << put_time(localtime(&now), "%F %T") << endl;
	return std::ctime(&time);
}
#include "logger.h"

std::once_flag Logger::s_OnceFlag;
std::mutex Logger::s_FileLock;
Logger* Logger::s_Instance = nullptr;
std::ofstream Logger::m_FileStream;
string Logger::s_FileName = "Logger.log"; // default file name
bool Logger::s_IsMetaData = true;

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

void Logger::ResetInstance()
{
	delete s_Instance;
	s_Instance = nullptr;
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

void Logger::SetLogLevel(const LogLevel& i_LogLevel)
{
	std::lock_guard<std::mutex> lock(s_FileLock);
	Logger::GetInstance()->m_LogLevel = i_LogLevel;
}

void Logger::SetLogType(const LogType& i_LogType)
{
	std::lock_guard<std::mutex> lock(s_FileLock);
	Logger::GetInstance()->m_LogType = i_LogType;
}

void Logger::SetLogMetaData(bool i_IsMetaDataOn)
{
	s_IsMetaData = i_IsMetaDataOn;
}

void Logger::Info(const string& i_Message)
{
	if (m_LogLevel >= Logger::LogLevel::Info)
	{
		outputLog(constructLog(c_InfoHeadline, i_Message));
	}
}

void Logger::Error(const string& i_Message)
{
	if (m_LogLevel >= Logger::LogLevel::Error)
	{
		outputLog(constructLog(c_ErrorHeadline, i_Message));
	}
}

void Logger::Critical(const string& i_Message)
{
	if (m_LogLevel >= Logger::LogLevel::Critical)
	{
		outputLog(constructLog(c_CriticalHeadline, i_Message));
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
		throw exception();
	}
}

string Logger::constructLog(const string& i_Headline, const string& i_Message)
{
	stringstream ss;
	string token;
	vector<string> tokens;
	size_t pos = 0;

	// split the filename, line, func name and message into seperate strings
	ss << i_Message;
	for (int i = 0; i < 3; i++)
	{
		getline(ss, token, '\n');
		tokens.push_back(token);
		pos += token.size() + 1; // +1 is for '\n'
	}
	// extract the message itself
	tokens.push_back(ss.str());
	tokens[3] = tokens[3].substr(pos);

	// remove the full path from filename
	tokens[0] = tokens[0].substr(tokens[0].find_last_of(R"(\)") + 1);

	// build the log with time, headline and metadata
	ss.str("");
	ss.clear();
	if (s_IsMetaData)
	{
		ss << getCurrentTime() << " " << i_Headline << " " << tokens[0] << " at " << tokens[1] << " line " << tokens[2] << ": " << tokens[3];
	}
	else // build only with the headline
	{
		ss << i_Headline << " " << tokens[3];
	}
	return ss.str();
}

void Logger::outputLog(const string& i_Message)
{
	// lock
	std::lock_guard<std::mutex> lock(s_FileLock);

	// output
	if (m_LogType == Logger::LogType::File)
	{
		// init filestream upon first writing
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
	//localtime_s(&t, &now_time);
	strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &t);
	return buffer;
}
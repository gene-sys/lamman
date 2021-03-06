// Logger.cpp
// Implementation of a multithread safe singleton logger class
#include "stdafx.h"
#include "Logger.h"
#include <stdexcept>


using namespace std;

const string Logger::kLogLevelDebug = "DEBUG";
const string Logger::kLogLevelInfo = "INFO";
const string Logger::kLogLevelError = "ERROR";

const char* const Logger::kLogFileName = "laman.log";

Logger* Logger::pInstance = nullptr;

mutex Logger::sMutex;

Logger& Logger::instance()
{
	static Cleanup cleanup;

	lock_guard<mutex> guard(sMutex);
	if (pInstance == nullptr)
		pInstance = new Logger();
	return *pInstance;
}

Logger::Cleanup::~Cleanup()
{
	lock_guard<mutex> guard(Logger::sMutex);
	delete Logger::pInstance;
	Logger::pInstance = nullptr;
}

Logger::~Logger()
{
	mOutputStream.close();
}

const std::string Logger::CurrentDateTime()
{
	time_t     now = time(NULL);
	struct tm  tstruct;
	char       buf[80];
	localtime_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

Logger::Logger()
{
	mOutputStream.open(kLogFileName, ios_base::app);
	if (!mOutputStream.good()) {
		throw runtime_error("Unable to initialize the Logger!");
	}
}

void Logger::log(const string& inMessage, const string& inLogLevel)
{
	lock_guard<mutex> guard(sMutex);
	logHelper(inMessage, inLogLevel);
}

void Logger::log(const vector<string>& inMessages, const string& inLogLevel)
{
	lock_guard<mutex> guard(sMutex);
	for (size_t i = 0; i < inMessages.size(); i++) {
		logHelper(inMessages[i], inLogLevel);
	}
}

void Logger::logHelper(const std::string& inMessage, const std::string& inLogLevel)
{
	mOutputStream << CurrentDateTime() << ":\t" << inLogLevel << ": " << inMessage << endl;
}
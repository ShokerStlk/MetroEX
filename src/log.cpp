#include "mycommon.h"
#include <Windows.h>
#include <sstream>
#include <fstream>
#include <cstdarg>

#ifndef NDEBUG
#define LOG_OUTPUT_TO_DEBUG     1
#else
#define LOG_OUTPUT_TO_DEBUG     0
#endif

#define LOG_OUTPUT_TO_STDOUT    0
#define LOG_OUTPUT_TO_FILE      1

static const CharString sLevels[] = {
    "[Info] ",
    "[Warning] ",
    "[Error] "
};

#if LOG_OUTPUT_TO_FILE
static std::ofstream sLogFile;
#endif

void LogOpen(fs::path& folder) {
#if LOG_OUTPUT_TO_FILE
    fs::path finalPath = folder / L"log.txt";
    sLogFile.open(finalPath, std::ifstream::out | std::ifstream::trunc);
#endif
}

void LogClose() {
#if LOG_OUTPUT_TO_FILE
    if (sLogFile.good()) {
        sLogFile.flush();
        sLogFile.close();
    }
#endif
}

void LogPrint(LogLevel level, const CharString& message) {
    std::stringstream s;

    s << sLevels[scast<size_t>(level)] << message << std::endl;

    CharString result = s.str();

#if LOG_OUTPUT_TO_DEBUG
    OutputDebugStringA(result.c_str());
#endif

#if LOG_OUTPUT_TO_FILE
    sLogFile << result;
#endif
}

void LogPrintF(LogLevel level, const char* format, ...) {
    static char str_t[4096];
    va_list args;
    va_start(args, format);
    vsprintf(str_t, format, args);
    va_end(args);

    LogPrint(level, str_t);
}


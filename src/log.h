#pragma once

enum class LogLevel {
    Info,
    Warning,
    Error
};

void LogOpen(fs::path& folder);
void LogClose();
void LogPrint(LogLevel level, const CharString& message);
void LogPrintF(LogLevel level, const char* format, ...);

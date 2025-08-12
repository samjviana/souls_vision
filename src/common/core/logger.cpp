//
// Created by PC-SAMUEL on 23/11/2024.
//
#define NOMINMAX
#include "logger.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <windows.h>

namespace souls_vision {

std::ofstream Logger::logFile_;
std::mutex Logger::logMutex_;
bool Logger::debugMode_;

void Logger::Initialize(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(logMutex_);

    if (logFile_.is_open()) {
        logFile_.close();
    }

    logFile_.open(logFilePath, std::ios::out | std::ios::trunc);
    if (!logFile_) {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
    }
}

void Logger::InitializeDebug() {
    debugMode_ = true;
    InitializeConsole();
}

void Logger::Info(const std::string& message) {
    Log("INFO", message);
}

void Logger::Warning(const std::string& message) {
    Log("WARNING", message);
}

void Logger::Error(const std::string& message) {
    Log("ERROR", message);
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(logMutex_);

    if (logFile_.is_open()) {
        logFile_.close();
    }

    if (debugMode_) {
        FreeConsole();
    }
}

void Logger::Log(const std::string& severity, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex_);

    if (!logFile_.is_open()) {
        std::cerr << "Log file not initialized or failed to open." << std::endl;
        return;
    }

    std::time_t now = std::time(nullptr);
    std::tm localTime{};
    localtime_s(&localTime, &now);

    std::ostringstream timeStream;
    timeStream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");

    std::ostringstream logStream;
    logStream << "[" << timeStream.str() << "] [" << severity << "] " << message;

    if (logFile_.is_open()) {
        logFile_ << logStream.str() << std::endl;
    }

    if (debugMode_) {
        std::cout << logStream.str() << std::endl;
    }
}

void Logger::InitializeConsole() {
    if (!AllocConsole()) {
        std::cerr << "Failed to allocate console." << std::endl;
        return;
    }

    FILE* consoleStream = nullptr;
    freopen_s(&consoleStream, "CONOUT$", "w", stdout);
    std::cout.clear();

    freopen_s(&consoleStream, "CONOUT$", "w", stderr);
    std::cerr.clear();

    freopen_s(&consoleStream, "CONIN$", "r", stdin);
    std::clog.clear();
}

} // souls_vision
//
// Created by PC-SAMUEL on 23/11/2024.
//

#ifndef SOULS_VISION_LOGGER_H
#define SOULS_VISION_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

namespace souls_vision {

class Logger {
public:
    static void Initialize(const std::string& logFilePath);
    static void InitializeDebug();

    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);

    static void Shutdown();

private:
    static void Log(const std::string& severity, const std::string& message);
    static void InitializeConsole();

    static std::ofstream logFile_;
    static std::mutex logMutex_;
    static bool debugMode_;
};

} // souls_vision

#endif //SOULS_VISION_LOGGER_H

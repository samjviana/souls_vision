#define NOMINMAX
#include "globals.h"
#include "game_handler.h"
#include "hook_helper.h"
#include "overlay.h"
#include "logger.h"
#include "config.h"

#include <thread>
#include <iostream>

using namespace souls_vision;

DWORD WINAPI Setup(LPVOID lpParam);
void Cleanup();
void MainThread();
std::string GetDllPath(HMODULE hModule);
std::string GetDllDirectory(HMODULE hModule);
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
Size GetWindowSize(HWND hwnd = nullptr);

DWORD WINAPI Setup(LPVOID lpParam) {
    auto hModule = static_cast<HMODULE>(lpParam);
    gModule = hModule;

    gDllPath = GetDllDirectory(hModule);
    gConfigFilePath = gDllPath + "\\sv_config.json";

    std::string logFilePath = gDllPath + "\\souls_vision.log";
    Logger::Initialize(logFilePath);
    Logger::Info("Starting SoulsVision...");

    while (!gGameWindow) {
        gGameWindow = FindWindowW(nullptr, gWindowClass);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    gGameWindowSize = GetWindowSize(gGameWindow);

    Config::LoadConfig(gConfigFilePath);
    Config::SaveConfig(gConfigFilePath);
    if (Config::debug) {
        Logger::InitializeDebug();
    }

    GameHandler::Initialize();
    while (!souls_vision::GameHandler::CSMenuManImp() || souls_vision::GameHandler::CSMenuManImp()->loadingScreenData.timer <= 0.0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    Logger::Info("Game initialized and loaded");

    HookHelper::Hook();
    Overlay::Initialize();

    MainThread();

    Cleanup();
    FreeLibraryAndExitThread(hModule, 0);
}

void Cleanup() {
    Logger::Info("Shutting down SoulsVision...");
    Overlay::Uninitialize();
    HookHelper::Unhook();

    Logger::Info("SoulsVision shutdown complete");
    Logger::Shutdown();
}

void MainThread() {
    gRunning = true;

    std::string configPath = gDllPath + "\\sv_config.json";
    auto lastWriteTime = std::filesystem::last_write_time(configPath);

    Logger::Info("Main thread started");
    while (gRunning) {
        try {
            auto currentWriteTime = std::filesystem::last_write_time(configPath);
            if (currentWriteTime != lastWriteTime) {
                lastWriteTime = currentWriteTime;

                Logger::Info("Config file updated. Reloading...");
                Config::LoadConfig(configPath);
                Config::configUpdated = true;
            }
        } catch (const std::exception& e) {
            Logger::Error(std::string("Error checking config file: ") + e.what());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::string GetDllPath(HMODULE hModule) {
    char path[MAX_PATH];
    if (GetModuleFileNameA(hModule, path, MAX_PATH) == 0) {
        return "";
    }

    return {path};
}

std::string GetDllDirectory(HMODULE hModule) {
    std::string path = GetDllPath(hModule);
    size_t lastSlashIndex = path.find_last_of("\\/");
    if (lastSlashIndex == std::string::npos) {
        return "";
    }

    return path.substr(0, lastSlashIndex);
}

Size GetWindowSize(HWND hwnd) {
    if (!hwnd) {
        hwnd = FindWindowW(nullptr, gWindowClass);
    }

    RECT rect;
    if (GetClientRect(hwnd, &rect)) {
        auto width = rect.right - rect.left;
        auto height = rect.bottom - rect.top;
        Logger::Info("Window size: " + std::to_string(width) + "x" + std::to_string(height));
        return {width, height};
    } else {
        Logger::Error("Failed to get window size");
        return {0, 0}; // Return zero size if the function fails
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        CreateThread(nullptr, 0, Setup, hModule, 0, nullptr);
    } else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        gRunning = false;
    }

    return TRUE;
}

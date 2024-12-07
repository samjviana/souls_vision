//
// Created by PC-SAMUEL on 22/11/2024.
//

#ifndef SOULS_VISION_GLOBALS_H
#define SOULS_VISION_GLOBALS_H
#include <windows.h>

#include <atomic>
#include <cstdint>
#include <string>
#include "imgui.h"

namespace souls_vision {

struct Size {
    int width;
    int height;
};

extern const wchar_t* gWindowClass;
extern HMODULE gModule;
extern HWND gGameWindow;
extern Size gGameWindowSize;
extern std::string gDllPath;
extern std::string gConfigFilePath;
extern std::atomic_bool gRunning;
extern bool gRendered;

}

#endif //SOULS_VISION_GLOBALS_H

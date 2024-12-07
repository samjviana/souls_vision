#include "globals.h"

namespace souls_vision {

const wchar_t* gWindowClass = L"ELDEN RING\u2122";
HMODULE gModule = nullptr;
HWND gGameWindow = nullptr;
Size gGameWindowSize;
std::string gDllPath;
std::string gConfigFilePath;
std::atomic_bool gRunning = false;
bool gRendered = false;

}

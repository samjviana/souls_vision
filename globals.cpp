#include "globals.h"

namespace souls_vision {

const wchar_t* gWindowClass = L"ELDEN RING\u2122";
HWND gGameWindow = nullptr;
Size gGameWindowSize;
std::string gDllPath;
std::atomic_bool gRunning = false;

}

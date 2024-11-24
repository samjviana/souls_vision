//
// Created by PC-SAMUEL on 22/11/2024.
//

#ifndef SOULS_VISION_MEMORY_H
#define SOULS_VISION_MEMORY_H
#define NOMINMAX
#include <windows.h>
#include <string>

namespace souls_vision {

class Memory {
public:
    static uintptr_t SignatureScan(const std::string& signature, const std::string& moduleName);
    static uintptr_t DereferenceAddress(uintptr_t address);
    static void PrintMemoryBytes(uintptr_t address, size_t size);
    static std::string ToString(uintptr_t address);
};

} // souls_vision

#endif //SOULS_VISION_MEMORY_H

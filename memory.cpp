//
// Created by PC-SAMUEL on 22/11/2024.
//

#include "memory.h"

#include <vector>
#include <sstream>
#include <psapi.h>
#include <cstdint>
#include <iostream>
#include <iomanip>

namespace souls_vision {

uintptr_t Memory::SignatureScan(const std::string& signature, const std::string& moduleName) {
    HMODULE hModule = GetModuleHandleA(moduleName.c_str());
    if (!hModule) {
        return 0;
    }

    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(MODULEINFO))) {
        return 0;
    }

    auto startAddress = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
    DWORD moduleSize = modInfo.SizeOfImage;

    std::vector<uint8_t> patternBytes;
    std::vector<bool> patternMask;
    std::istringstream iss(signature);
    std::string token;

    while (iss >> token) {
        if (token == "?" || token == "??") {
            patternBytes.push_back(0);
            patternMask.push_back(false);
        } else {
            patternBytes.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, 16)));
            patternMask.push_back(true);
        }
    }

    size_t patternLength = patternBytes.size();

    for (uintptr_t i = startAddress; i <= startAddress + moduleSize - patternLength; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternLength; ++j) {
            if (patternMask[j] && *(uint8_t*)(i + j) != patternBytes[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return i;
        }
    }
    return 0;
}

uintptr_t Memory::DereferenceAddress(uintptr_t address) {
    address += 3;
    int offset = *reinterpret_cast<int*>(address);
    address += offset + 4;
    address = *reinterpret_cast<uintptr_t*>(address);

    return address;
}

void Memory::PrintMemoryBytes(uintptr_t address, size_t size) {
    auto ptr = reinterpret_cast<unsigned char*>(address);

    std::cout << "0x" << std::hex << address << std::dec << ": " << std::endl;

    for (size_t i = 0; i < size; ++i) {
        std::cout << "\t" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(ptr[i]) << " ";

        if ((i + 1) % 8 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

std::string Memory::ToString(uintptr_t address) {
    std::stringstream ss;
    ss << std::hex << address;
    return ss.str();
}

} // souls_vision
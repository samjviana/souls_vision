//
// Created by PC-SAMUEL on 25/11/2024.
//

#include "util.h"

#include "resources.h"

namespace souls_vision {

D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart, SIZE_T descriptorIncrementSize, int index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = {};
    handle.ptr = srvHeapStart.ptr + descriptorIncrementSize * index;
    return handle;
}

BOOL CALLBACK EnumResourceNamesA(HMODULE hModule, LPCSTR lpType, LPSTR lpName, LONG_PTR lParam) {
    int* pCount = reinterpret_cast<int*>(lParam);
    (*pCount)++;
    return TRUE;
}

int CountPngResources(HMODULE hModule) {
    int resourceCount = 0;

    EnumResourceNames(hModule, MAKEINTRESOURCE(RT_PNG), EnumResourceNamesA, reinterpret_cast<LONG_PTR>(&resourceCount));

    return resourceCount;
}

bool ModuleExists(const std::string& moduleName) {
    HMODULE hModule = GetModuleHandleA(moduleName.c_str());
    return hModule != nullptr;
}

} // souls_vision
//
// Created by PC-SAMUEL on 25/11/2024.
//

#include "util.h"

namespace souls_vision {

D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart, SIZE_T descriptorIncrementSize, int index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = {};
    handle.ptr = srvHeapStart.ptr + descriptorIncrementSize * index;
    return handle;
}

BOOL CALLBACK EnumResourceNamesA(HMODULE hModule, LPCSTR lpType, LPSTR lpName, LONG_PTR lParam) {
    if (!IS_INTRESOURCE(lpName)) {
        return TRUE;
    }

    int resourceID = LOWORD(reinterpret_cast<ULONG_PTR>(lpName));

    const int MIN_PNG_ID = 201;
    const int MAX_PNG_ID = 299;

    if (resourceID >= MIN_PNG_ID && resourceID <= MAX_PNG_ID) {
        int* pCount = reinterpret_cast<int*>(lParam);
        (*pCount)++;
    }

    return TRUE;
}

int CountPngResources(HMODULE hModule) {
    int resourceCount = 0;

    EnumResourceNames(hModule, RT_RCDATA, EnumResourceNamesA, reinterpret_cast<LONG_PTR>(&resourceCount));

    return resourceCount;
}

} // souls_vision
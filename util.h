//
// Created by PC-SAMUEL on 25/11/2024.
//

#ifndef SOULS_VISION_UTIL_H
#define SOULS_VISION_UTIL_H

#include <d3d12.h>

namespace souls_vision {

extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart, SIZE_T descriptorIncrementSize, int index);
extern int CountPngResources(HMODULE hModule);

} // souls_vision

#endif //SOULS_VISION_UTIL_H

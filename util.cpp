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

} // souls_vision
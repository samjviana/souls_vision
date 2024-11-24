//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_BAR_RENDERER_H
#define SOULS_VISION_BAR_RENDERER_H

#include <d3d12.h>
#include <imgui.h>
#include "overlay.h"

namespace souls_vision {

struct BarSettings {
    ImVec2 position;
    ImVec2 size;
    float currentValue = 0;
    float maxValue = 1;
    bool hideText;
};

class BarRenderer {
public:
    BarRenderer(ID3D12Device* device, D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart, SIZE_T descriptorIncrementSize);

    void Render(ImDrawList* drawList, const BarSettings& settings, const TextureInfo& barTexture, int decimals = 0);

private:
    ID3D12Device* device_;
    D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart_;
    SIZE_T descriptorIncrementSize_;
    TextureInfo backgroundInfo_;
    TextureInfo edgeInfo_;
    TextureInfo frameInfo_;

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int index);
};


} // souls_vision

#endif //SOULS_VISION_BAR_RENDERER_H

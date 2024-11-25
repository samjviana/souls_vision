//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_EFFECT_BAR_RENDERER_H
#define SOULS_VISION_EFFECT_BAR_RENDERER_H

#include <d3d12.h>
#include <imgui.h>
#include "bar_renderer.h"

namespace souls_vision {

class EffectBarRenderer {
public:
    EffectBarRenderer(ID3D12Device* device, D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart, SIZE_T descriptorIncrementSize);

    void Render(ImDrawList* drawList, const BarSettings& settings, const TextureInfo& effectIconInfo, ImU32 barColor, int decimals = 0);

private:
    ID3D12Device* device_;
    D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart_;
    SIZE_T descriptorIncrementSize_;
    TextureInfo backgroundInfo_;
    TextureInfo barInfo_;
    TextureInfo edgeInfo_;
    TextureInfo frameInfo_;

    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int index) const;
};

} // souls_vision

#endif //SOULS_VISION_EFFECT_BAR_RENDERER_H

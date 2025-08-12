//
// Created by PC-SAMUEL on 27/11/2024.
//

#ifndef SOULS_VISION_STAT_BAR_H
#define SOULS_VISION_STAT_BAR_H

#include <string>
#include "shared_types.h"

namespace souls_vision {

class StatBar {
public:
    StatBar(BarType type, const std::string& barTexture, SIZE_T descriptorSize, D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart);

    void Render(const BarSettings& settings, float paddingX, float paddingY, int decimals = 0);

    BarType type;

private:
    TextureInfo backgroundInfo_;
    TextureInfo barInfo_;
    TextureInfo edgeInfo_;
    TextureInfo frameInfo_;
    D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart_;
    SIZE_T descriptorIncrementSize_;
};

} // souls_vision

#endif //SOULS_VISION_STAT_BAR_H

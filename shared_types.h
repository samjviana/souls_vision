//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_SHARED_TYPES_H
#define SOULS_VISION_SHARED_TYPES_H

#include <d3d12.h>
#include <imgui.h>

namespace souls_vision {

enum class BarType {
    HP,
    FP,
    Stamina,
    Stagger,
    Poison,
    ScarletRot,
    Hemorrhage,
    DeathBlight,
    Frostbite,
    Sleep,
    Madness
};

struct TextureInfo {
    ID3D12Resource* textureResource = nullptr;
    int index = -1;
    int width = 0;
    int height = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle;
};

struct BarSettings {
    ImVec2 position;
    ImVec2 size;
    float currentValue = 0;
    float maxValue = 1;
    bool hideText;
};

} // souls_vision

#endif //SOULS_VISION_SHARED_TYPES_H

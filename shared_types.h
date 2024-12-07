//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_SHARED_TYPES_H
#define SOULS_VISION_SHARED_TYPES_H

#include <d3d12.h>
#include <imgui.h>
#include <string>

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
    std::string textureName;
};

struct ComponentVisibility {
    bool hp = true;
    bool fp = true;
    bool stamina = true;
    bool stagger = true;
    bool poison = true;
    bool scarletRot = true;
    bool hemorrhage = true;
    bool deathBlight = true;
    bool frostbite = true;
    bool sleep = true;
    bool madness = true;
    bool bestEffects = true;
    bool immuneEffects = true;
    bool dmgTypes = true;
    bool neutralDmgTypes = false;
};

struct BarConfig {
    BarType type;
    float currentValue;
    float maxValue;
    const char* textureName;
    ImColor barColor = IM_COL32(255, 255, 255, 255);
    int decimals = 0;
    bool isEffect = false;
    bool condition = true;
};

struct BarToRender {
    BarSettings settings;
    TextureInfo textureInfo;
    BarConfig config;
    ImU32 barColor;
    int decimals = 0;
};

} // souls_vision

#endif //SOULS_VISION_SHARED_TYPES_H

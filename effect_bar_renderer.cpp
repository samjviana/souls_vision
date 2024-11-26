//
// Created by PC-SAMUEL on 24/11/2024.
//

#define NOMINMAX
#include "effect_bar_renderer.h"
#include "logger.h"
#include "config.h"
#include "overlay.h"
#include "util.h"

namespace souls_vision {

EffectBarRenderer::EffectBarRenderer(ID3D12Device* device, D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart, SIZE_T descriptorIncrementSize)
        : device_(device), srvHeapStart_(srvHeapStart), descriptorIncrementSize_(descriptorIncrementSize) {
    backgroundInfo_ = Overlay::GetTexture("BarBG.png");
    barInfo_ = Overlay::GetTexture("Bar.png");
    edgeInfo_ = Overlay::GetTexture("BarEdge2.png");
    frameInfo_ = Overlay::GetTexture("ConditionWaku.png");

    if (!backgroundInfo_.textureResource || !barInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.textureResource) {
        Logger::Error("Failed to load default textures.");
    }
}

void EffectBarRenderer::Render(ImDrawList* drawList, const BarSettings& settings, const TextureInfo& effectIconInfo, ImU32 barColor, int decimals) {
    if (!effectIconInfo.textureResource || !backgroundInfo_.textureResource || !barInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.textureResource) {
        Logger::Error("Failed to retrieve one or more textures.");
        return;
    }

    // Calculate descriptor handles
    D3D12_GPU_DESCRIPTOR_HANDLE backgroundHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, backgroundInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE barHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, barInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE edgeHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, edgeInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE frameHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, frameInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE effectIconHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, effectIconInfo.index);

    auto backgroundTexID = static_cast<ImTextureID>(backgroundHandle.ptr);
    auto barTexID = static_cast<ImTextureID>(barHandle.ptr);
    auto edgeTexID = static_cast<ImTextureID>(edgeHandle.ptr);
    auto frameTexID = static_cast<ImTextureID>(frameHandle.ptr);
    auto effectIconTexID = static_cast<ImTextureID>(effectIconHandle.ptr);

    ImVec2 iconSize = Config::effectBarIconSize;

    ImVec2 framePosition = settings.position;
    framePosition.x += iconSize.x - (iconSize.x * 0.142f);
    framePosition.y += (iconSize.y / 2) - (settings.size.y / 2);

    ImVec2 frameSize = settings.size;
    frameSize.x -= iconSize.x;
    ImVec2 barPosition = ImVec2(framePosition.x, framePosition.y + (frameSize.y * 0.125f));
    ImVec2 barSize = ImVec2(frameSize.x - 5, frameSize.y - (frameSize.y * 0.3f));

    float maxValue = std::max(settings.maxValue, 1.0f);
    float percentage = settings.currentValue / maxValue;

    // Draw background
    ImVec2 backgroundEnd = ImVec2(barPosition.x + barSize.x, barPosition.y + barSize.y);
    ImU32 tintColor = IM_COL32(255, 255, 255, 255);
    drawList->AddImage(backgroundTexID, barPosition, backgroundEnd, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tintColor);

    // Draw bar, clipped by percentage
    float clipWidth = barSize.x * percentage;
    ImVec2 barEnd = ImVec2(barPosition.x + clipWidth, barPosition.y + barSize.y);
    drawList->AddImage(barTexID, barPosition, barEnd, ImVec2(0.0f, 0.0f), ImVec2(percentage, 1.0f), barColor);

    // Draw edge clip to avoid overlapping
    ImVec2 clipStart = barPosition;
    ImVec2 clipEnd = barEnd;
    drawList->PushClipRect(clipStart, clipEnd, true);

    // Draw edge
    ImVec2 edgePosition = ImVec2(barEnd.x - (float)edgeInfo_.width + 8, barPosition.y - 2);
    ImVec2 edgeSize = ImVec2(edgePosition.x + (float)edgeInfo_.width, edgePosition.y + barSize.y - 1);
    drawList->AddImage(edgeTexID, edgePosition, edgeSize);

    drawList->PopClipRect();

    // Draw frame
    ImVec2 frameEnd = ImVec2(framePosition.x + frameSize.x, framePosition.y + frameSize.y);
    drawList->AddImage(frameTexID, framePosition, frameEnd);

    // Draw effect icon
    ImVec2 iconPosition = settings.position;
    ImVec2 iconEnd = ImVec2(iconPosition.x + iconSize.x, iconPosition.y + iconSize.y);
    drawList->AddImage(effectIconTexID, iconPosition, iconEnd);

    // Draw text in the center
    if (settings.hideText) {
        return;
    }
    std::string text = std::format("{:.{}f} / {:.{}f}", settings.currentValue, decimals, settings.maxValue, decimals);
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 textPosition = ImVec2(
            barPosition.x + 10,
            framePosition.y + (frameSize.y - textSize.y) / 2
    );
    drawList->AddText(textPosition, IM_COL32(255, 255, 255, 255), text.c_str());
}

} // souls_vision
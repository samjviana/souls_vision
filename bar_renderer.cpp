//
// Created by PC-SAMUEL on 24/11/2024.
//

#define NOMINMAX
#include <iostream>
#include "bar_renderer.h"
#include "logger.h"

namespace souls_vision {

BarRenderer::BarRenderer(ID3D12Device* device, D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart, SIZE_T descriptorIncrementSize)
    : device_(device), srvHeapStart_(srvHeapStart), descriptorIncrementSize_(descriptorIncrementSize) {
    backgroundInfo_ = Overlay::GetTexture("BarBG.png");
    edgeInfo_ = Overlay::GetTexture("BarEdge2.png");
    frameInfo_ = Overlay::GetTexture("BuddyWaku.png");

    if (!backgroundInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.textureResource) {
        Logger::Error("Failed to load default textures.");
    }
}

void BarRenderer::Render(ImDrawList* drawList, const BarSettings& settings, const TextureInfo& barTexture, int decimals) {
    if (!barTexture.textureResource || !backgroundInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.textureResource) {
        Logger::Error("Failed to retrieve one or more textures.");
        return;
    }

    // Calculate descriptor handles
    D3D12_GPU_DESCRIPTOR_HANDLE backgroundHandle = GetGpuDescriptorHandle(backgroundInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE barHandle = GetGpuDescriptorHandle(barTexture.index);
    D3D12_GPU_DESCRIPTOR_HANDLE edgeHandle = GetGpuDescriptorHandle(edgeInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE frameHandle = GetGpuDescriptorHandle(frameInfo_.index);

    ImTextureID backgroundTexID = static_cast<ImTextureID>(backgroundHandle.ptr);
    ImTextureID barTexID = static_cast<ImTextureID>(barHandle.ptr);
    ImTextureID edgeTexID = static_cast<ImTextureID>(edgeHandle.ptr);
    ImTextureID frameTexID = static_cast<ImTextureID>(frameHandle.ptr);

    ImVec2 framePosition = settings.position;
    ImVec2 frameSize = settings.size;
    ImVec2 barPosition = ImVec2(framePosition.x + 16, framePosition.y + 5);
    ImVec2 barSize = ImVec2(frameSize.x - 33, frameSize.y - 11);

    float maxValue = std::max((float)settings.maxValue, 1.0f);
    float percentage = (float)settings.currentValue / maxValue;

    // Draw background
    ImVec2 backgroundEnd = ImVec2(barPosition.x + barSize.x, barPosition.y + barSize.y);
    ImU32 tintColor = IM_COL32(255, 255, 255, 255);
    drawList->AddImage(backgroundTexID, barPosition, backgroundEnd, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tintColor);

    // Draw bar, clipped by percentage
    float clipWidth = barSize.x * percentage;
    ImVec2 barEnd = ImVec2(barPosition.x + clipWidth, barPosition.y + barSize.y);
    drawList->AddImage(barTexID, barPosition, barEnd, ImVec2(0.0f, 0.0f), ImVec2(percentage, 1.0f));

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

    // Draw text in the center
    if (settings.hideText) {
        return;
    }
    std::string text = std::format("{:.{}f} / {:.{}f}", settings.currentValue, decimals, settings.maxValue, decimals);
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 textPosition = ImVec2(
            framePosition.x + (frameSize.x - textSize.x) / 2,
            framePosition.y + (frameSize.y - textSize.y) / 2
    );
    drawList->AddText(textPosition, IM_COL32(255, 255, 255, 255), text.c_str());
}

D3D12_GPU_DESCRIPTOR_HANDLE BarRenderer::GetGpuDescriptorHandle(int index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = {};
    handle.ptr = srvHeapStart_.ptr + descriptorIncrementSize_ * index;
    return handle;
}

} // souls_vision
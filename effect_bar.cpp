//
// Created by PC-SAMUEL on 27/11/2024.
//

#include "effect_bar.h"
#include "overlay.h"
#include "util.h"
#include "logger.h"
#include "config.h"

namespace souls_vision {

    EffectBar::EffectBar(BarType _type, const std::string& iconPath, ImVec4 barColor, SIZE_T descriptorSize, D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart) {
    type = _type;

    backgroundInfo_ = Overlay::GetTexture("BarBG.png");
    barInfo_ = Overlay::GetTexture("Bar.png");
    edgeInfo_ = Overlay::GetTexture("BarEdge2.png");
    frameInfo_ = Overlay::GetTexture("ConditionWaku.png");
    iconInfo_ = Overlay::GetTexture(iconPath);
    barColor_ = barColor;

    srvHeapStart_ = srvHeapStart;
    descriptorIncrementSize_ = descriptorSize;

    if (!backgroundInfo_.textureResource || !barInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.textureResource || !iconInfo_.textureResource) {
        Logger::Error("Failed to load default textures.");
    }
}

void EffectBar::Render(const BarSettings &settings, float paddingX, float paddingY, int decimals) {
    if (!backgroundInfo_.textureResource || !barInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.textureResource || !iconInfo_.textureResource) {
        Logger::Error("Failed to retrieve one or more textures.");
        return;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE backgroundHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, backgroundInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE barHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, barInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE edgeHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, edgeInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE frameHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, frameInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE iconHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, iconInfo_.index);

    auto backgroundTexID = static_cast<ImTextureID>(backgroundHandle.ptr);
    auto barTexID = static_cast<ImTextureID>(barHandle.ptr);
    auto edgeTexID = static_cast<ImTextureID>(edgeHandle.ptr);
    auto frameTexID = static_cast<ImTextureID>(frameHandle.ptr);
    auto iconTexID = static_cast<ImTextureID>(iconHandle.ptr);

    float percentage = settings.currentValue / settings.maxValue;

    ImVec2 iconSize = Config::effectBarIconSize;

    ImVec2 barSize = ImVec2(
            settings.size.x * 0.876,
            settings.size.y * 0.73
    );
    ImVec2 barPosition = ImVec2(
            paddingX + (iconSize.x - (iconSize.x * 0.14)),
            paddingY + (iconSize.y / 2) - (barSize.y / 2)
    );
    ImVec2 uv0 = ImVec2(0.0f, 0.0f);
    ImVec2 uv1 = ImVec2(percentage, 1.0f);

    ImGui::SetCursorPos(barPosition);
    ImVec4 tintColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    ImGui::Image(backgroundTexID, barSize, uv0, ImVec2(1.0f, 1.0f), tintColor);

    ImGui::SetCursorPos(barPosition);
    ImGui::Image(barTexID, ImVec2(barSize.x * percentage, barSize.y), uv0, uv1, barColor_);

    ImVec2 clipMin = ImVec2(
            settings.position.x + barPosition.x,
            settings.position.y + barPosition.y
    );
    ImVec2 clipMax = ImVec2(
            settings.position.x + settings.size.x + paddingX,
            settings.position.y + settings.size.y + paddingY
    );
    ImGui::PushClipRect(clipMin, clipMax, true);
    ImVec2 edgeSize = ImVec2(
            (float)settings.size.x * 0.2f,
            (float)settings.size.y * 0.75f
    );
    ImVec2 edgePosition = ImVec2(
            barPosition.x + (barSize.x * percentage) - (edgeSize.x * 0.92f),
            barPosition.y - (edgeSize.y * 0.1f)
    );
    ImGui::SetCursorPos(ImVec2(edgePosition));
    ImGui::Image(edgeTexID, edgeSize);
    ImGui::PopClipRect();

    ImVec2 framePosition = ImVec2(
            paddingX + iconSize.x - (iconSize.x * 0.14),
            paddingY + (iconSize.y / 2) - (settings.size.y / 2)
    );
    ImVec2 frameSize = ImVec2(
            settings.size.x - iconSize.x,
            settings.size.y
    );
    ImGui::SetCursorPos(framePosition);
    ImGui::Image(frameTexID, frameSize);

    ImVec2 iconPosition = ImVec2(paddingX, paddingY);
    ImGui::SetCursorPos(iconPosition);
    ImGui::Image(iconTexID, iconSize);

    if (!settings.hideText) {
        ImGui::PushFont(Overlay::font_);

        std::string text = std::format("{:.{}f} / {:.{}f}", settings.currentValue, decimals, settings.maxValue, decimals);
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImVec2 textPosition = ImVec2(
                barPosition.x + 10.0f,
                paddingY + ((iconSize.y - textSize.y) * 0.5f)
        );
        ImGui::SetCursorPos(textPosition);
        ImGui::Text("%s", text.c_str());

        ImGui::PopFont();
    }
}

} // souls_vision
//
// Created by PC-SAMUEL on 27/11/2024.
//
#define NOMINMAX

#include "effect_bar.h"
#include <format>
#include "overlay.h"
#include "util.h"
#include "logger.h"
#include "config.h"
#include "globals.h"

namespace souls_vision {
    EffectBar::EffectBar(BarType _type, const std::string &iconPath, ImVec4 barColor, SIZE_T descriptorSize,
                         D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart) {
        type = _type;

        backgroundInfo_ = Overlay::GetTexture("BarBG.png");
        barInfo_ = Overlay::GetTexture("Bar.png");
        edgeInfo_ = Overlay::GetTexture("BarEdge2.png");
        frameInfo_ = Overlay::GetTexture("ConditionWaku.png");
        iconInfo_ = Overlay::GetTexture(iconPath);
        barColor_ = barColor;

        srvHeapStart_ = srvHeapStart;
        descriptorIncrementSize_ = descriptorSize;

        if (!backgroundInfo_.textureResource || !barInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.
            textureResource || !iconInfo_.textureResource) {
            Logger::Error("Failed to load default textures.");
        }
    }

    void EffectBar::Render(const BarSettings &barSettings, float paddingX, float paddingY, int decimals) {
        if (!backgroundInfo_.textureResource || !barInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.
            textureResource || !iconInfo_.textureResource) {
            Logger::Error("Failed to retrieve one or more textures.");
            return;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE backgroundHandle = GetGpuDescriptorHandle(
            srvHeapStart_, descriptorIncrementSize_, backgroundInfo_.index);
        D3D12_GPU_DESCRIPTOR_HANDLE barHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_,
                                                                       barInfo_.index);
        D3D12_GPU_DESCRIPTOR_HANDLE edgeHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_,
                                                                        edgeInfo_.index);
        D3D12_GPU_DESCRIPTOR_HANDLE frameHandle = GetGpuDescriptorHandle(
            srvHeapStart_, descriptorIncrementSize_, frameInfo_.index);
        D3D12_GPU_DESCRIPTOR_HANDLE iconHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_,
                                                                        iconInfo_.index);

        auto backgroundTexID = static_cast<ImTextureID>(backgroundHandle.ptr);
        auto barTexID = static_cast<ImTextureID>(barHandle.ptr);
        auto edgeTexID = static_cast<ImTextureID>(edgeHandle.ptr);
        auto frameTexID = static_cast<ImTextureID>(frameHandle.ptr);
        auto iconTexID = static_cast<ImTextureID>(iconHandle.ptr);

        /*
         * Basic Bar / Icon
         */
        float iconSize = Config::statBarSettings.iconSize;
        ImVec2 barSize = ImVec2(
            barSettings.size.x - iconSize,
            barSettings.size.y
        );
        ImVec2 iconPosition = ImVec2(
            paddingX,
            iconSize > barSize.y
                ? paddingY
                : paddingY + barSize.y * .5f - iconSize * .5f
        );
        float barXOffset = barSize.x * .01;
        ImVec2 barPosition = ImVec2(
            iconPosition.x + iconSize - barXOffset,
            iconSize > barSize.y
                ? paddingY + iconSize * .5f - barSize.y * .5f
                : paddingY
        );
        barSize.x *= 1.01;

        /* Percentage of status buildup */
        float percentage = barSettings.currentValue / barSettings.maxValue;
        ImVec2 uv0 = ImVec2(0.0f, 0.0f);
        ImVec2 uv1 = ImVec2(percentage, 1.0f);
        ImVec2 uv1Full = ImVec2(1.0f, 1.0f);

        /*
         * Black Background
         */
        float blackBgWidthRatio = 1.f;
        float blackBgHeightRatio = 26.f / 36.f; // BarBG.png height /  ConditionWaku.png height
        ImVec2 blackBgSize = ImVec2(
            barSize.x * blackBgWidthRatio,
            barSize.y * blackBgHeightRatio
        );
        ImVec2 blackBgPos = ImVec2(
            barPosition.x + (barSize.x - blackBgSize.x) * .5f,
            barPosition.y + (barSize.y - blackBgSize.y) * .5f
        );
        ImGui::SetCursorPos(blackBgPos);
        ImVec4 tintColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        ImGui::Image(backgroundTexID, blackBgSize, uv0, uv1Full, tintColor);

        /*
         * Colored Bar
         */
        float coloredBarWidthRatio = 1.f;
        float coloredBarHeightRatio = 26.f / 36.f; // Bar.png height /  ConditionWaku.png height
        float coloredBarFullWidth = barSize.x * coloredBarWidthRatio;
        ImVec2 coloredBarSize = ImVec2(
            coloredBarFullWidth * percentage,
            barSize.y * coloredBarHeightRatio
        );
        ImVec2 coloredBarPosition = ImVec2(
            barPosition.x + (barSize.x - coloredBarFullWidth) * .5f,
            barPosition.y + (barSize.y - coloredBarSize.y) * .5f
        );
        ImGui::SetCursorPos(coloredBarPosition);
        ImGui::Image(barTexID, coloredBarSize, uv0, uv1, barColor_);

        /*
         * White Tick/Trail at End of Colored Bar
         */
        ImVec2 edgeSize = ImVec2(
            barSize.x * 0.2f,
            barSize.y
        );
        float edgeXOffset = edgeSize.x * .07f;
        ImVec2 edgePosition = ImVec2(
            coloredBarPosition.x + coloredBarSize.x - edgeSize.x + edgeXOffset,
            barPosition.y + (barSize.y - edgeSize.y) * .5f
        );
        /* ClipRect keeps tick/trail from leaking outside of bar */
        ImVec2 clipMin = ImVec2(
            barPosition.x,
            0
        );
        ImVec2 clipMax = ImVec2(
            coloredBarPosition.x + coloredBarFullWidth * .995f,
            99999999.f
        );
        ImGui::PushClipRect(clipMin, clipMax, true);
        ImGui::SetCursorPos(edgePosition);
        ImGui::Image(edgeTexID, edgeSize);
        ImGui::PopClipRect();

        /*
         * Frame/Border
         */
        ImVec2 framePosition = ImVec2(
            barPosition.x,
            barPosition.y
        );
        ImVec2 frameSize = ImVec2(
            barSize.x,
            barSize.y
        );
        ImGui::SetCursorPos(framePosition);
        ImGui::Image(frameTexID, frameSize);

        /*
         * Text
         */
        ImGui::SetCursorPos(iconPosition);
        ImGui::Image(iconTexID, ImVec2(iconSize, iconSize));;
        if (!barSettings.hideText) {
            ImGui::PushFont(Overlay::font_);
            std::string text = std::format("{:.{}f} / {:.{}f}", barSettings.currentValue, decimals,
                                           barSettings.maxValue,
                                           decimals);
            ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
            ImVec2 textPosition = ImVec2(
                barPosition.x + (barSize.x - textSize.x) * .5f,
                barPosition.y + (barSize.y - textSize.y) * .5f
            );
            ImGui::SetCursorPos(textPosition);
            ImGui::Text("%s", text.c_str());

            ImGui::PopFont();
        }
    }
} // souls_vision

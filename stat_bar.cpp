//
// Created by PC-SAMUEL on 27/11/2024.
//

#include "stat_bar.h"
#include "overlay.h"
#include "logger.h"
#include "util.h"

namespace souls_vision {

StatBar::StatBar(BarType _type, const std::string& barTexture, SIZE_T descriptorSize, D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart) {
    type = _type;

    backgroundInfo_ = Overlay::GetTexture("BarBG.png");
    barInfo_ = Overlay::GetTexture(barTexture);
    edgeInfo_ = Overlay::GetTexture("BarEdge2.png");
    frameInfo_ = Overlay::GetTexture("BuddyWaku.png");

    srvHeapStart_ = srvHeapStart;
    descriptorIncrementSize_ = descriptorSize;

    if (!backgroundInfo_.textureResource || !barInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.textureResource) {
        Logger::Error("Failed to load default textures.");
    }
}

void StatBar::Render(const BarSettings &settings, float paddingX, float paddingY, int decimals) {
    if (!backgroundInfo_.textureResource || !barInfo_.textureResource || !edgeInfo_.textureResource || !frameInfo_.textureResource) {
        Logger::Error("Failed to retrieve one or more textures.");
        return;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE backgroundHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, backgroundInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE barHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, barInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE edgeHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, edgeInfo_.index);
    D3D12_GPU_DESCRIPTOR_HANDLE frameHandle = GetGpuDescriptorHandle(srvHeapStart_, descriptorIncrementSize_, frameInfo_.index);

    auto backgroundTexID = static_cast<ImTextureID>(backgroundHandle.ptr);
    auto barTexID = static_cast<ImTextureID>(barHandle.ptr);
    auto edgeTexID = static_cast<ImTextureID>(edgeHandle.ptr);
    auto frameTexID = static_cast<ImTextureID>(frameHandle.ptr);

    float percentage = settings.currentValue / settings.maxValue;

    ImVec2 barPosition = ImVec2(
            paddingX,
            paddingY
    );
    ImVec2 barSize = ImVec2(
            settings.size.x,
            settings.size.y
    );
    ImVec2 uv0 = ImVec2(0.0f, 0.0f);
    ImVec2 uv1 = ImVec2(percentage, 1.0f);
    ImVec2 uv1Full = ImVec2(1.0f, 1.0f);

    ImGui::SetCursorPos(barPosition);
    ImVec4 tintColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    ImGui::Image(backgroundTexID, barSize, uv0, uv1Full, tintColor);

    ImGui::SetCursorPos(barPosition);
    ImGui::Image(barTexID, ImVec2(barSize.x * percentage, barSize.y), uv0, uv1);

    ImVec2 clipMin = ImVec2(
            barPosition.x,
            0
    );
    ImVec2 clipMax = ImVec2(
            barPosition.x + barSize.x * .995f,
            99999999.f
    );
    ImGui::PushClipRect(clipMin, clipMax, true);
    ImVec2 edgeSize = ImVec2(
            barSize.x * 0.2f,
            barSize.y * .8f
    );
    float edgeXOffset = edgeSize.x * .07f;
    ImVec2 edgePosition = ImVec2(
            barPosition.x + barSize.x * percentage - edgeSize.x + edgeXOffset,
            barPosition.y + (barSize.y - edgeSize.y) * .5f
    );
    ImGui::SetCursorPos(ImVec2(edgePosition));
    ImGui::Image(edgeTexID, edgeSize);
    ImGui::PopClipRect();

    ImVec2 frameSize = ImVec2(
        barSize.x * 1.03963f, /* Account for transparency of png. About 3.963% of width is transparent */
        barSize.y * 1.27f   /* Account for transparency of png. About 27% of height is transparent */
    );
    ImVec2 framePosition = ImVec2(
        barPosition.x - frameSize.x * .02f,
        barPosition.y - frameSize.y * .09f
    );
    ImGui::SetCursorPos(framePosition);
    ImGui::Image(frameTexID, frameSize);

    if (!settings.hideText) {
        ImGui::PushFont(Overlay::font_);

        std::string text = std::format("{:.{}f} / {:.{}f}", settings.currentValue, decimals, settings.maxValue, decimals);
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImVec2 textPosition = ImVec2(
                barPosition.x + ((barSize.x - textSize.x) * 0.5f),
                barPosition.y + ((barSize.y - textSize.y) * 0.5f)
        );
        ImGui::SetCursorPos(textPosition);
        ImGui::Text("%s", text.c_str());

        ImGui::PopFont();
    }
}

} // souls_vision
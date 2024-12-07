//
// Created by PC-SAMUEL on 22/11/2024.
//

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#define NOMINMAX
#include "overlay.h"
#include "hook_helper.h"
#include "game_handler.h"
#include "globals.h"
#include "logger.h"
#include "stb_image.h"
#include "config.h"
#include "util.h"
#include "memory.h"
#include "resources.h"

#include <string>
#include <algorithm>
#include <ranges>

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

#include <iostream>
#include <set>

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace souls_vision {

uint64_t Overlay::oWndProc_ = 0;

ID3D12DescriptorHeap* Overlay::srvHeap_ = nullptr;
ID3D12DescriptorHeap* Overlay::rtvHeap_ = nullptr;
ID3D12CommandAllocator** Overlay::commandAllocators_ = nullptr;
ID3D12GraphicsCommandList* Overlay::commandList_ = nullptr;
ID3D12CommandQueue* Overlay::commandQueue_ = nullptr;
ID3D12Resource** Overlay::renderTargets_ = nullptr;
std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Overlay::cpuDescriptorHandles_;
uint64_t Overlay::buffersCounts_ = 0;
ImFont* Overlay::font_;

int Overlay::textureCount_;
std::unordered_map<std::string, TextureInfo> Overlay::textureMap_;

std::vector<StatBar> Overlay::statBars_;
std::vector<EffectBar> Overlay::effectBars_;
std::vector<BarToRender> Overlay::barsToRender_;
std::vector<BarToRender> Overlay::effectBarsToRender_;
std::vector<std::string> Overlay::effectsNames = {
        "Poison",
        "ScarletRot",
        "Hemorrhage",
        "DeathBlight",
        "Frostbite",
        "Sleep",
        "Madness"
};

void Overlay::Initialize() {
    HookHelper::SetRenderCallback(Render);
    HookHelper::SetCleanupCallback(CleanupRenderTargets);

    textureCount_ = GetTextureCount();

    if (ImGui::GetCurrentContext()) {
        return;
    }

    if (!ImGui::CreateContext()) {
        Logger::Error("Failed to create ImGui context");
        return;
    }
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;

    gGameWindow = FindWindowW(nullptr, gWindowClass);
    if (!gGameWindow) {
        Logger::Error("Failed to find game window: ELDEN RING\u2122");
        return;
    }

    if (!ImGui_ImplWin32_Init(gGameWindow)) {
        Logger::Error("Failed to initialize ImGui for Win32");
        return;
    }

    Logger::Info("Overlay initialized successfully.");
}

int Overlay::GetTextureCount() {
    const std::string folderPath = gDllPath + "\\sv_assets\\";

    if (!std::filesystem::exists(folderPath)) {
        Logger::Error("Folder not found: " + folderPath);
        return 0;
    }

    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        auto filePath = entry.path().string();
        if (filePath.ends_with(".png") || filePath.ends_with(".jpg")) {
            count++;
        }

        for (const auto& effect : effectsNames) {
            if (filePath.ends_with(effect + ".png") || filePath.ends_with(effect + ".jpg")) {
                count++;
            }
        }
    }



    return count;
}

void Overlay::Uninitialize() {
    if (!ImGui::GetCurrentContext()) {
        return;
    }

    if (ImGui::GetIO().BackendPlatformUserData) {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
    }
    ImGui::DestroyContext();
    Logger::Info("ImGui platforms shutdown and context destroyed.");

    CleanupRenderTargets();

    statBars_.clear();
    effectBars_.clear();
    barsToRender_.clear();
    effectBarsToRender_.clear();

    Logger::Info("Overlay uninitialized successfully.");
}

void Overlay::InitializeDXResources(IDXGISwapChain3 *pSwapChain) {
    DXGI_SWAP_CHAIN_DESC sd;
    pSwapChain->GetDesc(&sd);

    if (ImGui::GetIO().BackendRendererUserData) {
        return;
    }

    ID3D12Device* device;
    if (pSwapChain->GetDevice(IID_PPV_ARGS(&device)) != S_OK)
        return;

    buffersCounts_ = sd.BufferCount;

    cpuDescriptorHandles_.clear();

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = textureCount_ + 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap_)) != S_OK) {
            device->Release();
            Logger::Error("Failed to create SRV heap.");
            return;
        }
    }
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = static_cast<UINT>(buffersCounts_);
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap_)) != S_OK) {
            srvHeap_->Release();
            device->Release();
            return;
        }

        const SIZE_T rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
        commandAllocators_ = new ID3D12CommandAllocator *[buffersCounts_];
        for (int i = 0; i < buffersCounts_; ++i) {
            cpuDescriptorHandles_.push_back(rtvHandle);
            rtvHandle.ptr += rtvDescriptorSize;
        }
        device->Release();
    }

    for (UINT i = 0; i < sd.BufferCount; ++i) {
        if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                           IID_PPV_ARGS(&commandAllocators_[i])) != S_OK) {
            srvHeap_->Release();
            for (UINT j = 0; j < i; ++j) {
                commandAllocators_[j]->Release();
            }
            rtvHeap_->Release();
            delete[] commandAllocators_;
            device->Release();
            return;
        }
    }

    if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators_[0], nullptr,
                                  IID_PPV_ARGS(&commandList_)) != S_OK ||
        commandList_->Close() != S_OK) {
        srvHeap_->Release();
        for (UINT i = 0; i < buffersCounts_; ++i)
            commandAllocators_[i]->Release();
        rtvHeap_->Release();
        delete[] commandAllocators_;
        device->Release();
        return;
    }

    ImGui_ImplDX12_Init(device, static_cast<int>(buffersCounts_),
                        DXGI_FORMAT_R8G8B8A8_UNORM, srvHeap_,
                        srvHeap_->GetCPUDescriptorHandleForHeapStart(),
                        srvHeap_->GetGPUDescriptorHandleForHeapStart());
    ImGui::GetMainViewport()->PlatformHandleRaw = souls_vision::gGameWindow;
    oWndProc_ = SetWindowLongPtrW(souls_vision::gGameWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

void Overlay::InitializeFileResources(ID3D12Device* device) {
    if (!font_ || Config::fontSizeUpdated) {
        font_ = LoadFont();
        Config::fontSizeUpdated = false;
    }

    if (textureMap_.empty() || Config::opacityUpdated) {
        LoadAllTextures(device);
        Config::opacityUpdated = false;
    }
}

inline int Overlay::GetCorrectDXGIFormat(int eCurrentFormat) {
    if (eCurrentFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    return eCurrentFormat;
}

void Overlay::InitializeRenderTargets(IDXGISwapChain3 *pSwapChain) {
    DXGI_SWAP_CHAIN_DESC sd;
    pSwapChain->GetDesc(&sd);

    if (renderTargets_) {
        return;
    }

    ID3D12Device* device;
    if (pSwapChain->GetDevice(IID_PPV_ARGS(&device)) != S_OK)
        return;
    renderTargets_ = new ID3D12Resource *[buffersCounts_];
    for (UINT i = 0; i < buffersCounts_; i++) {
        ID3D12Resource *buffer;
        if (pSwapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)) != S_OK) {
            continue;
        }
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        device->CreateRenderTargetView(buffer, &desc, cpuDescriptorHandles_[i]);
        renderTargets_[i] = buffer;
    }
    if (renderTargets_[0] == nullptr) {
        for (UINT i = 0; i < buffersCounts_; ++i) {
            if (renderTargets_[i])
                renderTargets_[i]->Release();
        }
        delete[] renderTargets_;
        renderTargets_ = nullptr;
    }
    device->Release();
}

void Overlay::InitializeBars(ID3D12Device* device) {
    const SIZE_T descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (statBars_.empty()) {
        StatBar hpBar(BarType::HP, "Red.png", descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());
        StatBar fpBar(BarType::FP, "Blue.png", descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());
        StatBar staminaBar(BarType::Stamina, "Green.png", descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());
        StatBar staggerBar(BarType::Stagger, "Yellow.png", descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());

        statBars_.push_back(hpBar);
        statBars_.push_back(fpBar);
        statBars_.push_back(staminaBar);
        statBars_.push_back(staggerBar);
    }

    if (effectBars_.empty()) {
        EffectBar poisonBar(BarType::Poison, "Poison.png", GetColor0To1(100, 113, 0, 255), descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());
        EffectBar scarletRotBar(BarType::ScarletRot, "ScarletRot.png", GetColor0To1(117, 38, 1, 255), descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());
        EffectBar hemorrhageBar(BarType::Hemorrhage, "Hemorrhage.png", GetColor0To1(95, 11, 11, 255), descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());
        EffectBar deathBlightBar(BarType::DeathBlight, "DeathBlight.png", GetColor0To1(57, 53, 50, 255), descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());
        EffectBar frostbiteBar(BarType::Frostbite, "Frostbite.png", GetColor0To1(48, 95, 133, 255), descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());
        EffectBar sleepBar(BarType::Sleep, "Sleep.png", GetColor0To1(77, 80, 114, 255), descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());
        EffectBar madnessBar(BarType::Madness, "Madness.png", GetColor0To1(131, 85, 0, 255), descriptorSize, srvHeap_->GetGPUDescriptorHandleForHeapStart());

        effectBars_.push_back(poisonBar);
        effectBars_.push_back(scarletRotBar);
        effectBars_.push_back(hemorrhageBar);
        effectBars_.push_back(deathBlightBar);
        effectBars_.push_back(frostbiteBar);
        effectBars_.push_back(sleepBar);
        effectBars_.push_back(madnessBar);
    }
}

void Overlay::DrawStatBars(ID3D12Device* device) {
    ChrIns* localPlayer = GameHandler::GetLocalPlayer();
    if (!localPlayer || localPlayer->targetHandle == -1) {
        return;
    }

    ChrIns* targetChrIns = GameHandler::GetChrInsFromHandle(&localPlayer->targetHandle);
    if (!targetChrIns) {
        return;
    }

    NpcParam* npcParam = GameHandler::GetNpcParam(targetChrIns->paramId);
    if (!npcParam) {
        return;
    }

    std::unordered_map<std::string, float> dmgAbsorptions = {
            {"Fire", npcParam->fireDamageCutRate},
            {"Magic", npcParam->magicDamageCutRate},
            {"Lightning", npcParam->thunderDamageCutRate},
            {"Holy", npcParam->darkDamageCutRate},
    };
    std::vector<std::pair<std::string, float>> bestDmgTypes = dmgAbsorptions
            | std::views::filter([](const std::pair<std::string, float>& pair) { return pair.second > 1.0; })
            | std::views::transform([](const std::pair<std::string, float>& pair) { return pair; })
            | std::ranges::to<std::vector>();
    std::ranges::sort(bestDmgTypes, [](const std::pair<std::string, float>& a, const std::pair<std::string, float>& b) {
        return a.second > b.second;
    });
    std::vector<float> uniqueBests = bestDmgTypes
            | std::views::transform([](const std::pair<std::string, float>& pair) { return pair.second; })
            | std::ranges::to<std::set>()
            | std::views::transform([](float f) { return f; })
            | std::ranges::to<std::vector>();
    std::ranges::sort(uniqueBests, std::less<>());
    std::map<std::string, int> bestDmgTypeIndices = {};
    for (int i = 0; i < uniqueBests.size(); i++) {
        std::string twoPoint = std::format("{:.2f}", uniqueBests[i]);
        bestDmgTypeIndices[twoPoint] = i + 1;
    }
    int biggestBestIndex = 0;
    if (!uniqueBests.empty()) {
        std::string biggestBest = std::format("{:.2f}", std::ranges::max(uniqueBests));
        biggestBestIndex = bestDmgTypeIndices.contains(biggestBest) ? bestDmgTypeIndices[biggestBest] : -1;
    }

    std::vector<std::pair<std::string, float>> worseDmgTypes = dmgAbsorptions
            | std::views::filter([](const std::pair<std::string, float>& pair) { return pair.second < 1.0; })
            | std::views::transform([](const std::pair<std::string, float>& pair) { return pair; })
            | std::ranges::to<std::vector>();
    std::ranges::sort(worseDmgTypes, [](const std::pair<std::string, float>& a, const std::pair<std::string, float>& b) {
        return a.second < b.second;
    });
    std::vector<float> uniqueWorses = worseDmgTypes
            | std::views::transform([](const std::pair<std::string, float>& pair) { return pair.second; })
            | std::ranges::to<std::set>()
            | std::views::transform([](float f) { return f; })
            | std::ranges::to<std::vector>();
    std::ranges::sort(uniqueWorses, std::greater<>());
    std::map<std::string, int> worseDmgTypeIndices = {};
    for (int i = 0; i < uniqueWorses.size(); i++) {
        std::string twoPoint = std::format("{:.2f}", uniqueWorses[i]);
        worseDmgTypeIndices[twoPoint] = i + 1;
    }
    int biggestWorseIndex = 0;
    if (!uniqueWorses.empty()) {
        std::string biggestWorse = std::format("{:.2f}", std::ranges::max(uniqueWorses));
        biggestWorseIndex = worseDmgTypeIndices.contains(biggestWorse) ? worseDmgTypeIndices[biggestWorse] : -1;
    }

    std::vector<std::pair<std::string, float>> neutralDmgTypes = dmgAbsorptions
            | std::views::filter([](const std::pair<std::string, float>& pair) { return pair.second == 1.0; })
            | std::views::transform([](const std::pair<std::string, float>& pair) { return pair; })
            | std::ranges::to<std::vector>();

    int maxIconLength = std::max((int)bestDmgTypes.size(), (int)worseDmgTypes.size());
    maxIconLength = std::max(maxIconLength, (int)neutralDmgTypes.size());

    std::vector<BarType> immuneEffects = effectBars_
            | std::views::filter([&targetChrIns](const EffectBar& bar) { return GetTargetMaxValue(bar.type, targetChrIns) >= 999; })
            | std::views::transform([](const EffectBar& bar) { return bar.type; })
            | std::ranges::to<std::vector>();

    std::vector<BarType> strongestEffects = effectBars_
            | std::views::filter([&targetChrIns](const EffectBar& bar) { return GetTargetMaxValue(bar.type, targetChrIns) < 999; })
            | std::views::transform([](const EffectBar& bar) { return bar.type; })
            | std::ranges::to<std::vector>();

    std::ranges::sort(strongestEffects, [&targetChrIns](BarType a, BarType b) {
        return GetTargetMaxValue(a, targetChrIns) < GetTargetMaxValue(b, targetChrIns);
    });

    std::vector<std::tuple<StatBar, BarSettings, int>> statBarsToRender = {};
    for (auto bar : statBars_) {
        bool show = GetBarVisibility(bar.type);
        if (!show) {
            continue;
        }

        BarSettings settings = Config::statBarSettings;
        settings.currentValue = GetTargetValue(bar.type, targetChrIns);
        settings.maxValue = GetTargetMaxValue(bar.type, targetChrIns);
        int decimals = bar.type == BarType::Stagger ? 2 : 0;
        if (settings.maxValue <= 0) {
            continue;
        }

        statBarsToRender.emplace_back(bar, settings, decimals);
    }

    std::vector<std::tuple<EffectBar, BarSettings, int>> effectBarsToRender = {};
    for (auto bar : effectBars_) {
        bool show = GetBarVisibility(bar.type);
        if (!show) {
            continue;
        }

        BarSettings settings = Config::statBarSettings;
        settings.maxValue = GetTargetMaxValue(bar.type, targetChrIns);
        settings.currentValue = settings.maxValue - GetTargetValue(bar.type, targetChrIns);
        int decimals = 0;
        if (settings.maxValue <= 0 || settings.currentValue <= 0) {
            continue;
        }

        effectBarsToRender.emplace_back(bar, settings, decimals);
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar;

    if (!Config::dragOverlay) {
        windowFlags |= ImGuiWindowFlags_NoDecoration
                       | ImGuiWindowFlags_NoBackground
                       | ImGuiWindowFlags_NoInputs;
    } else {
        windowFlags |= ImGuiWindowFlags_NoResize;
    }

    float dmgTypesWidth = (Config::dmgTypeIconSize.x * maxIconLength);
    float windowWidth = Config::statBarSettings.size.x + dmgTypesWidth;
    ImVec2 windowPosition = Config::statBarSettings.position;
    if (gRendered) {
        windowPosition.x -= dmgTypesWidth;
    }

    float statBarsheight = ((Config::statBarSettings.size.y - 10.0f + Config::statBarSpacing) * statBarsToRender.size() + (10.0f + Config::statBarSpacing));
    float bestEffectHeight = 0;
    if (Config::bestEffects > 0) {
        bestEffectHeight = Config::bestEffectIconSize.y - 5.0f + (Config::statBarSpacing * 0.5f);
    }
    float effectBarsHeight = (Config::effectBarIconSize.y + Config::statBarSpacing) * effectBarsToRender.size();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); // Remove o padding
    ImVec2 windowSize = ImVec2(
            windowWidth,
            statBarsheight + bestEffectHeight + effectBarsHeight
    );
    ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y));

    static ImVec2 previousPosition = Config::statBarSettings.position;

    if (!ImGui::Begin("Stat Window", nullptr, windowFlags)) {
        return;
    }
    if (Config::configUpdated) {
        ImGui::SetWindowPos(Config::statBarSettings.position);
        previousPosition = Config::statBarSettings.position;
        Config::configUpdated = false;
    }

    ImVec2 currentPosition = ImGui::GetWindowPos();

    if (Config::dragOverlay && (currentPosition.x != previousPosition.x || currentPosition.y != previousPosition.y)) {
        Config::statBarSettings.position = currentPosition;
        previousPosition = currentPosition;
        Config::SaveConfig(gConfigFilePath);
    }

    for (int i = 0; i < statBarsToRender.size(); i++) {
        auto [bar, settings, decimals] = statBarsToRender[i];
        float paddingY = static_cast<float>(i) * (settings.size.y - 10.0f + Config::statBarSpacing);
        float paddingX = dmgTypesWidth;

        bar.Render(settings, paddingX, paddingY, decimals);
    }

    float paddingY = statBarsheight;

    for (int i = 0; i < Config::bestEffects && i < strongestEffects.size() && Config::componentVisibility.bestEffects; i++) {
        BarType type = strongestEffects[i];
        TextureInfo effectTexture = GetTexture(GetTextureNameForType(type));
        if (!effectTexture.textureResource) {
            continue;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE effectIconHandle = GetGpuDescriptorHandle(srvHeap_->GetGPUDescriptorHandleForHeapStart(), device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), effectTexture.index);
        auto effectIconTexID = static_cast<ImTextureID>(effectIconHandle.ptr);

        ImVec2 iconSize = Config::bestEffectIconSize;
        ImVec2 iconPosition = ImVec2(
            dmgTypesWidth + (Config::bestEffectIconSize.x * i) + 5.0f,
            paddingY - 5.0f + (Config::statBarSpacing * 0.5f)
        );
        ImGui::SetCursorPos(iconPosition);
        ImGui::Image(effectIconTexID, iconSize);
    }

    for (int i = 0; i < immuneEffects.size() && Config::componentVisibility.immuneEffects; i++) {
        BarType type = immuneEffects[i];
        TextureInfo effectTexture = GetTexture(GetTextureNameForType(type, true));
        if (!effectTexture.textureResource) {
            continue;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE effectIconHandle = GetGpuDescriptorHandle(srvHeap_->GetGPUDescriptorHandleForHeapStart(), device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), effectTexture.index);
        auto effectIconTexID = static_cast<ImTextureID>(effectIconHandle.ptr);

        ImVec2 iconSize = Config::bestEffectIconSize;
        ImVec2 iconPosition = ImVec2(
            (Config::statBarSettings.size.x + dmgTypesWidth) - ((immuneEffects.size() - i) * Config::bestEffectIconSize.x) - 10.0f,
            paddingY - 5.0f + (Config::statBarSpacing * 0.5f)
        );
        ImGui::SetCursorPos(iconPosition);
        ImGui::Image(effectIconTexID, iconSize);
    }

    if (!strongestEffects.empty() || Config::bestEffects != 0 || !immuneEffects.empty()) {
        paddingY += bestEffectHeight + Config::statBarSpacing;
    }

    for (int i = 0; i < effectBarsToRender.size(); i++) {
        auto [bar, settings, decimals] = effectBarsToRender[i];
        float innerPaddingY = paddingY + static_cast<float>(i) * (Config::effectBarIconSize.y + Config::statBarSpacing);
        float paddingX = dmgTypesWidth;

        bar.Render(settings, paddingX, innerPaddingY, decimals);
    }

    ImVec2 damageTypeIconSize = Config::dmgTypeIconSize;

    for (int i = 0; i < bestDmgTypes.size() && Config::componentVisibility.dmgTypes; i++) {
        auto [dmgType, value] = bestDmgTypes[i];
        TextureInfo dmgTypeTexture = GetTexture(dmgType + ".png");
        if (!dmgTypeTexture.textureResource) {
            continue;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE dmgTypeIconHandle = GetGpuDescriptorHandle(srvHeap_->GetGPUDescriptorHandleForHeapStart(), device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), dmgTypeTexture.index);
        auto dmgTypeIconTexID = static_cast<ImTextureID>(dmgTypeIconHandle.ptr);

        ImVec2 iconPosition = ImVec2(
            dmgTypesWidth - (damageTypeIconSize.x * (i + 1)) + (Config::statBarSettings.size.x * 0.01),
            (Config::statBarSettings.size.y / 2.0f) - (damageTypeIconSize.y / 2.0f)
        );
        ImGui::SetCursorPos(iconPosition);
        ImGui::Image(dmgTypeIconTexID, damageTypeIconSize);

        TextureInfo greenArrowTexture = GetTexture("GreenArrow.png");
        if (!greenArrowTexture.textureResource) {
            continue;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE greenArrowIconHandle = GetGpuDescriptorHandle(srvHeap_->GetGPUDescriptorHandleForHeapStart(), device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), greenArrowTexture.index);
        auto greenArrowIconTexID = static_cast<ImTextureID>(greenArrowIconHandle.ptr);

        ImVec2 arrowSize = ImVec2(damageTypeIconSize.x * 0.45, damageTypeIconSize.x * 0.8125 * 0.45);
        ImVec2 arrowPosition = iconPosition;
        arrowPosition.x += damageTypeIconSize.x - arrowSize.x + (arrowSize.x * 0.15);
        arrowPosition.y += damageTypeIconSize.y - arrowSize.y;

        std::string strValue = std::format("{:.2f}", value);
        if (bestDmgTypeIndices.contains(strValue)) {
            int arrowCount = bestDmgTypeIndices[strValue];

            for (int i = 0; i < arrowCount; i++) {
                arrowPosition.y -= arrowSize.y * i * 0.55;
                ImGui::SetCursorPos(arrowPosition);
                ImGui::Image(greenArrowIconTexID, arrowSize);
            }
        }
    }

    paddingY = damageTypeIconSize.y + Config::statBarSpacing;

    for (int i = 0; i < neutralDmgTypes.size() && Config::componentVisibility.dmgTypes && Config::componentVisibility.neutralDmgTypes; i++) {
        auto [dmgType, value] = neutralDmgTypes[i];
        TextureInfo dmgTypeTexture = GetTexture(dmgType + ".png");
        if (!dmgTypeTexture.textureResource) {
            continue;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE dmgTypeIconHandle = GetGpuDescriptorHandle(srvHeap_->GetGPUDescriptorHandleForHeapStart(), device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), dmgTypeTexture.index);
        auto dmgTypeIconTexID = static_cast<ImTextureID>(dmgTypeIconHandle.ptr);

        ImVec2 iconPosition = ImVec2(
            dmgTypesWidth - (damageTypeIconSize.x * (i + 1)) + (Config::statBarSettings.size.x * 0.01),
            paddingY + (Config::statBarSettings.size.y / 2.0f) - (damageTypeIconSize.y / 2.0f)
        );
        ImGui::SetCursorPos(iconPosition);
        ImGui::Image(dmgTypeIconTexID, damageTypeIconSize);
    }

    paddingY += damageTypeIconSize.y + Config::statBarSpacing;

    for (int i = 0; i < worseDmgTypes.size() && Config::componentVisibility.dmgTypes; i++) {
        auto [dmgType, value] = worseDmgTypes[i];
        TextureInfo dmgTypeTexture = GetTexture(dmgType + ".png");
        if (!dmgTypeTexture.textureResource) {
            continue;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE dmgTypeIconHandle = GetGpuDescriptorHandle(srvHeap_->GetGPUDescriptorHandleForHeapStart(), device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), dmgTypeTexture.index);
        auto dmgTypeIconTexID = static_cast<ImTextureID>(dmgTypeIconHandle.ptr);

        ImVec2 iconPosition = ImVec2(
            dmgTypesWidth - (damageTypeIconSize.x * (i + 1)) + (Config::statBarSettings.size.x * 0.01),
            paddingY + (Config::statBarSettings.size.y / 2.0f) - (damageTypeIconSize.y / 2.0f)
        );
        ImGui::SetCursorPos(iconPosition);
        ImGui::Image(dmgTypeIconTexID, damageTypeIconSize);

        TextureInfo redArrowTexture = GetTexture("RedArrow.png");
        if (!redArrowTexture.textureResource) {
            continue;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE redArrowIconHandle = GetGpuDescriptorHandle(srvHeap_->GetGPUDescriptorHandleForHeapStart(), device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), redArrowTexture.index);
        auto redArrowIconTexID = static_cast<ImTextureID>(redArrowIconHandle.ptr);

        ImVec2 arrowSize = ImVec2(damageTypeIconSize.x * 0.45, damageTypeIconSize.x * 0.8125 * 0.45);
        ImVec2 arrowPosition = iconPosition;
        arrowPosition.x += damageTypeIconSize.x - arrowSize.x + (arrowSize.x * 0.15);
        arrowPosition.y += damageTypeIconSize.y - arrowSize.y;

        std::string strValue = std::format("{:.2f}", value);
        if (worseDmgTypeIndices.contains(strValue)) {
            int arrowCount = worseDmgTypeIndices[strValue];

            for (int i = 0; i < arrowCount; i++) {
                arrowPosition.y -= arrowSize.y * i * 0.55;
                ImGui::SetCursorPos(arrowPosition);
                ImGui::Image(redArrowIconTexID, arrowSize);
            }
        }
    }

    ImGui::End();

    ImGui::PopStyleVar();

    gRendered = true;
}

void Overlay::RenderTargets(IDXGISwapChain3 *pSwapChain) {
    UINT bufferIndex = pSwapChain->GetCurrentBackBufferIndex();
    ID3D12CommandAllocator *commandAllocator = commandAllocators_[bufferIndex];
    commandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = renderTargets_[bufferIndex];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    commandList_->Reset(commandAllocator, nullptr);
    commandList_->ResourceBarrier(1, &barrier);
    commandList_->OMSetRenderTargets(1, &cpuDescriptorHandles_[bufferIndex], FALSE, nullptr);
    commandList_->SetDescriptorHeaps(1, &srvHeap_);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commandList_->ResourceBarrier(1, &barrier);
    commandList_->Close();

    commandQueue_->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList **>(&commandList_));
}

void Overlay::Render(IDXGISwapChain3 *pSwapChain) {
    if (commandQueue_ == nullptr) {
        return;
    }

    ID3D12Device* device;
    if (pSwapChain->GetDevice(IID_PPV_ARGS(&device)) != S_OK) {
        Logger::Error("Failed to get ID3D12Device from SwapChain.");
        return;
    }

    InitializeDXResources(pSwapChain);
    InitializeFileResources(device);
    InitializeRenderTargets(pSwapChain);

    if (!ImGui::GetCurrentContext()) {
        device->Release();
        return;
    }

    InitializeBars(device);

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    DrawStatBars(device);

    ImGui::EndFrame();

    RenderTargets(pSwapChain);
}

void Overlay::CleanupRenderTargets() {
    if (!renderTargets_) return;
    for (UINT i = 0; i < buffersCounts_; ++i) {
        if (renderTargets_[i]) {
            renderTargets_[i]->Release();
            renderTargets_[i] = nullptr;
        }
    }
    delete[] renderTargets_;
    renderTargets_ = nullptr;
}

void Overlay::LoadAllTextures(ID3D12Device* device) {
    const std::string folderPath = gDllPath + "\\sv_assets\\";

    if (!std::filesystem::exists(folderPath)) {
        Logger::Error("Folder not found: " + folderPath);
        return;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = srvHeap_->GetCPUDescriptorHandleForHeapStart();
    SIZE_T descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    srvCpuHandle.ptr += descriptorSize;  // Adjust pointer to start at index 1

    int textureIndex = 1;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        auto filePath = entry.path().string();
        auto fileName = entry.path().filename().string();

        // This might be able to handle .dds files, but I'll use only .png and .jpg for now
        if (filePath.ends_with(".png") || filePath.ends_with(".jpg")) {
            TextureInfo textureInfo = {};
            if (LoadTextureFromFile(filePath.c_str(), device, srvCpuHandle, &textureInfo)) {
                textureInfo.index = textureIndex++;
                textureMap_[fileName] = textureInfo;
                Logger::Info(std::format("Loaded texture: {}", fileName));

                srvCpuHandle.ptr += descriptorSize;

                if (std::ranges::any_of(effectsNames, [&fileName](const std::string& effect) { return fileName.ends_with(effect + ".png") || fileName.ends_with(effect + ".jpg"); })) {
                    TextureInfo grayscaleTextureInfo = {};
                    if (LoadTextureFromFile(filePath.c_str(), device, srvCpuHandle, &grayscaleTextureInfo, true)) {
                        grayscaleTextureInfo.index = textureIndex++;
                        fileName = fileName.substr(0, fileName.find_last_of('.')) + "GrayScale" + fileName.substr(fileName.find_last_of('.'));
                        textureMap_[fileName] = grayscaleTextureInfo;
                        Logger::Info(std::format("Loaded texture: {}", fileName));

                        srvCpuHandle.ptr += descriptorSize;
                    }
                }
            } else {
                Logger::Error("Texture " + fileName + " failed to load");
            }
        }
    }
}

bool Overlay::LoadTextureFromFile(const char* fileName, ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle, TextureInfo* textureInfo, bool grayscale) {
    FILE* file = fopen(fileName, "rb");
    if (!file) {
        Logger::Error("Failed to open file: " + std::string(fileName));
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    if (fileSize == -1) {
        Logger::Error("Failed to get file size: " + std::string(fileName));
        fclose(file);
        return false;
    }

    fseek(file, 0, SEEK_SET);
    void* fileData = IM_ALLOC(fileSize);
    fread(fileData, 1, fileSize, file);
    bool ret = LoadTextureFromMemory(fileData, fileSize, device, srvCpuHandle, textureInfo, Config::opacity, grayscale);
    IM_FREE(fileData);

    return ret;
}

bool Overlay::LoadTextureFromMemory(const void* data, size_t data_size, ID3D12Device* d3d_device, D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle, TextureInfo* textureInfo, float opacity, bool grayscale) {
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL) {
        return false;
    }

    if (opacity < 0.0f) opacity = 0.0f;
    if (opacity > 1.0f) opacity = 1.0f;

    for (int y = 0; y < image_height; y++) {
        for (int x = 0; x < image_width; x++) {
            int index = (y * image_width + x) * 4;

            if (grayscale) {
                unsigned char r = image_data[index];
                unsigned char g = image_data[index + 1];
                unsigned char b = image_data[index + 2];

                // Compute grayscale value using luminance formula
                unsigned char gray = static_cast<unsigned char>(0.299f * r + 0.587f * g + 0.114f * b);

                image_data[index] = gray;
                image_data[index + 1] = gray;
                image_data[index + 2] = gray;
            }

            image_data[index + 3] = static_cast<unsigned char>(image_data[index + 3] * opacity);
        }
    }

    // Create texture resource
    D3D12_HEAP_PROPERTIES props;
    memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = image_width;
    desc.Height = image_height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* pTexture = NULL;
    d3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
                                        D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&pTexture));

    // Create a temporary upload resource to move the data in
    UINT uploadPitch = (image_width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
    UINT uploadSize = image_height * uploadPitch;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = uploadSize;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    props.Type = D3D12_HEAP_TYPE_UPLOAD;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    ID3D12Resource* uploadBuffer = NULL;
    HRESULT hr = d3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
                                                     D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uploadBuffer));
    IM_ASSERT(SUCCEEDED(hr));

    // Write pixels into the upload resource
    void* mapped = NULL;
    D3D12_RANGE range = { 0, uploadSize };
    hr = uploadBuffer->Map(0, &range, &mapped);
    IM_ASSERT(SUCCEEDED(hr));
    for (int y = 0; y < image_height; y++)
        memcpy((void*)((uintptr_t)mapped + y * uploadPitch), image_data + y * image_width * 4, image_width * 4);
    uploadBuffer->Unmap(0, &range);

    // Copy the upload resource content into the real resource
    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = uploadBuffer;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srcLocation.PlacedFootprint.Footprint.Width = image_width;
    srcLocation.PlacedFootprint.Footprint.Height = image_height;
    srcLocation.PlacedFootprint.Footprint.Depth = 1;
    srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = pTexture;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = pTexture;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    // Create a temporary command queue to do the copy with
    ID3D12Fence* fence = NULL;
    hr = d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    IM_ASSERT(SUCCEEDED(hr));

    HANDLE event = CreateEvent(0, 0, 0, 0);
    IM_ASSERT(event != NULL);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 1;

    ID3D12CommandQueue* cmdQueue = NULL;
    hr = d3d_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
    IM_ASSERT(SUCCEEDED(hr));

    ID3D12CommandAllocator* cmdAlloc = NULL;
    hr = d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
    IM_ASSERT(SUCCEEDED(hr));

    ID3D12GraphicsCommandList* cmdList = NULL;
    hr = d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, NULL, IID_PPV_ARGS(&cmdList));
    IM_ASSERT(SUCCEEDED(hr));

    cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
    cmdList->ResourceBarrier(1, &barrier);

    hr = cmdList->Close();
    IM_ASSERT(SUCCEEDED(hr));

    // Execute the copy
    cmdQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);
    hr = cmdQueue->Signal(fence, 1);
    IM_ASSERT(SUCCEEDED(hr));

    // Wait for everything to complete
    fence->SetEventOnCompletion(1, event);
    WaitForSingleObject(event, INFINITE);

    // Tear down our temporary command queue and release the upload resource
    cmdList->Release();
    cmdAlloc->Release();
    cmdQueue->Release();
    CloseHandle(event);
    fence->Release();
    uploadBuffer->Release();

    // Create a shader resource view for the texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    d3d_device->CreateShaderResourceView(pTexture, &srvDesc, srv_cpu_handle);

    if (!pTexture || pTexture == 0) {
        return false;
    }

    textureInfo->textureResource = pTexture;
    textureInfo->width = image_width;
    textureInfo->height = image_height;

    stbi_image_free(image_data);
    return true;
}

TextureInfo Overlay::GetTexture(const std::string &textureName) {
    // Safely accesses a texture from its name
    if (textureMap_.find(textureName) != textureMap_.end()) {
        return textureMap_[textureName];
    }

    Logger::Error("Texture not found: " + textureName);
    return {};
}

std::string Overlay::GetTextureNameForType(BarType type, bool grayscale) {
    std::unordered_map<BarType, std::string> textureNames = {
        { BarType::HP, "Red.png" },
        { BarType::FP, "Blue.png" },
        { BarType::Stamina, "Green.png" },
        { BarType::Stagger, "Yellow.png" },
        { BarType::Poison, "Poison.png" },
        { BarType::ScarletRot, "ScarletRot.png" },
        { BarType::Hemorrhage, "Hemorrhage.png" },
        { BarType::DeathBlight, "DeathBlight.png" },
        { BarType::Frostbite, "Frostbite.png" },
        { BarType::Sleep, "Sleep.png" },
        { BarType::Madness, "Madness.png" }
    };

    if (textureNames.find(type) == textureNames.end() && !grayscale) {
        return "";
    }

    if (!grayscale) {
        return textureNames[type];
    }

    textureNames = {
            { BarType::HP, "RedGrayScale.png" },
            { BarType::FP, "BlueGrayScale.png" },
            { BarType::Stamina, "GreenGrayScale.png" },
            { BarType::Stagger, "YellowGrayScale.png" },
            { BarType::Poison, "PoisonGrayScale.png" },
            { BarType::ScarletRot, "ScarletRotGrayScale.png" },
            { BarType::Hemorrhage, "HemorrhageGrayScale.png" },
            { BarType::DeathBlight, "DeathBlightGrayScale.png" },
            { BarType::Frostbite, "FrostbiteGrayScale.png" },
            { BarType::Sleep, "SleepGrayScale.png" },
            { BarType::Madness, "MadnessGrayScale.png" }
    };

    return textureNames[type];
}

LRESULT WINAPI Overlay::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    }

    return CallWindowProcW(reinterpret_cast<WNDPROC>(oWndProc_), hWnd, uMsg, wParam, lParam);
}

float Overlay::GetTargetValue(BarType type, ChrIns *targetChrIns) {
    if (!targetChrIns) {
        return 0;
    }

    if (type == BarType::HP) {
        return targetChrIns->moduleBag->statModule->hp;
    } else if (type == BarType::FP) {
        return targetChrIns->moduleBag->statModule->fp;
    } else if (type == BarType::Stamina) {
        return targetChrIns->moduleBag->statModule->stamina;
    } else if (type == BarType::Stagger) {
        return targetChrIns->moduleBag->superArmorModule->stagger;
    } else if (type == BarType::Poison) {
        return targetChrIns->moduleBag->resistModule->poisonResist;
    } else if (type == BarType::ScarletRot) {
        return targetChrIns->moduleBag->resistModule->scarletRotResist;
    } else if (type == BarType::Hemorrhage) {
        return targetChrIns->moduleBag->resistModule->hemorrhageResist;
    } else if (type == BarType::DeathBlight) {
        return targetChrIns->moduleBag->resistModule->deathBlightResist;
    } else if (type == BarType::Frostbite) {
        return targetChrIns->moduleBag->resistModule->frostbiteResist;
    } else if (type == BarType::Sleep) {
        return targetChrIns->moduleBag->resistModule->sleepResist;
    } else if (type == BarType::Madness) {
        return targetChrIns->moduleBag->resistModule->madnessResist;
    }
}

float Overlay::GetTargetMaxValue(BarType type, ChrIns *targetChrIns) {
    if (!targetChrIns) {
        return 0;
    }

    if (type == BarType::HP) {
        return targetChrIns->moduleBag->statModule->maxHp;
    } else if (type == BarType::FP) {
        return targetChrIns->moduleBag->statModule->maxFp;
    } else if (type == BarType::Stamina) {
        return targetChrIns->moduleBag->statModule->maxStamina;
    } else if (type == BarType::Stagger) {
        return targetChrIns->moduleBag->superArmorModule->maxStagger;
    } else if (type == BarType::Poison) {
        return targetChrIns->moduleBag->resistModule->maxPoisonResist;
    } else if (type == BarType::ScarletRot) {
        return targetChrIns->moduleBag->resistModule->maxScarletRotResist;
    } else if (type == BarType::Hemorrhage) {
        return targetChrIns->moduleBag->resistModule->maxHemorrhageResist;
    } else if (type == BarType::DeathBlight) {
        return targetChrIns->moduleBag->resistModule->maxDeathBlightResist;
    } else if (type == BarType::Frostbite) {
        return targetChrIns->moduleBag->resistModule->maxFrostbiteResist;
    } else if (type == BarType::Sleep) {
        return targetChrIns->moduleBag->resistModule->maxSleepResist;
    } else if (type == BarType::Madness) {
        return targetChrIns->moduleBag->resistModule->maxMadnessResist;
    }
}

bool Overlay::GetBarVisibility(BarType type) {
    if (type == BarType::HP) {
        return Config::componentVisibility.hp;
    } else if (type == BarType::FP) {
        return Config::componentVisibility.fp;
    } else if (type == BarType::Stamina) {
        return Config::componentVisibility.stamina;
    } else if (type == BarType::Stagger) {
        return Config::componentVisibility.stagger;
    } else if (type == BarType::Poison) {
        return Config::componentVisibility.poison;
    } else if (type == BarType::ScarletRot) {
        return Config::componentVisibility.scarletRot;
    } else if (type == BarType::Hemorrhage) {
        return Config::componentVisibility.hemorrhage;
    } else if (type == BarType::DeathBlight) {
        return Config::componentVisibility.deathBlight;
    } else if (type == BarType::Frostbite) {
        return Config::componentVisibility.frostbite;
    } else if (type == BarType::Sleep) {
        return Config::componentVisibility.sleep;
    } else if (type == BarType::Madness) {
        return Config::componentVisibility.madness;
    }
}

ImVec4 Overlay::GetColor0To1(int r, int g, int b, int a) {
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

ImFont* Overlay::LoadFont() {
    HRSRC hResource = FindResource(gModule, MAKEINTRESOURCE(IDR_AGMENA_W1G_FONT), RT_RCDATA);
    if (!hResource) {
        Logger::Error(std::format("Failed to find font resource. Resource ID: {}", IDR_AGMENA_W1G_FONT));
        return nullptr;
    }

    HGLOBAL hLoadedResource = LoadResource(gModule, hResource);
    if (!hLoadedResource) {
        Logger::Error("Failed to load font resource.");
        return nullptr;
    }

    void* pFontData = LockResource(hLoadedResource);
    if (!pFontData) {
        Logger::Error("Failed to lock font resource.");
        return nullptr;
    }

    DWORD fontSize = SizeofResource(gModule, hResource);
    if (fontSize == 0) {
        Logger::Error("Font size is zero.");
        return nullptr;
    }

    DWORD nFonts = 0;
    HANDLE hFont = AddFontMemResourceEx(pFontData, fontSize, NULL, &nFonts);
    if (!hFont) {
        Logger::Error("Failed to load font into system memory.");
        return nullptr;
    }

    ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(pFontData, fontSize, Config::fontSize);
    if (!font) {
        Logger::Error("Failed to add font to ImGui.");
        return nullptr;
    }
    ImGui::GetIO().Fonts->Build();
    ImGui_ImplDX12_InvalidateDeviceObjects();
    Logger::Info("Font added to ImGui successfully.");

    return font;
}

} // souls_vision
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
#include "structs/world_chr_man_imp.h"

#include <string>
#include <algorithm>

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

#include <iostream>

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace structs;

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

int Overlay::textureCount_;
std::unordered_map<std::string, TextureInfo> Overlay::textureMap_;

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

    LoadAllTextures(device);
}

inline int Overlay::GetCorrectDXGIFormat(int eCurrentFormat) {
    if (eCurrentFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    return eCurrentFormat;
}

void Overlay::InitializeRenderTargers(IDXGISwapChain3 *pSwapChain) {
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

void Overlay::Draw(ID3D12Device* device) {
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ChrIns* localPlayer = GameHandler::GetLocalPlayer();
    if (!localPlayer || localPlayer->targetHandle == -1) {
        return;
    }

    ChrIns* targetChrIns = GameHandler::GetChrInsFromHandle(&localPlayer->targetHandle);
    if (!targetChrIns) {
        return;
    }

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    const SIZE_T descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    BarRenderer barRenderer(device, srvHeap_->GetGPUDescriptorHandleForHeapStart(), descriptorSize);
    EffectBarRenderer effectBarRenderer(device, srvHeap_->GetGPUDescriptorHandleForHeapStart(), descriptorSize);

    std::vector<BarToRender> barsToRender;
    std::vector<EffectBarToRender> effectBarsToRender;

    auto AddStatusBar = [&](BarType type, float currentValue, float maxValue, const std::string& texturePath, int decimals = 0) {
        if (currentValue > maxValue || maxValue <= 0.0f) {
            return;
        }

        BarSettings barSettings = Config::defaultBarSettings;
        barSettings.currentValue = currentValue;
        barSettings.maxValue = maxValue;

        barsToRender.push_back(BarToRender{ barSettings, GetTexture(texturePath), decimals });
    };

    auto AddEffectBar = [&](BarType type, float currentValue, float maxValue, const std::string& texturePath, ImU32 color, int decimals = 0) {
        if (currentValue <= 0.0f || maxValue <= 0.0f) {
            return;
        }

        BarSettings effectSettings = Config::effectBarSettings;
        effectSettings.currentValue = maxValue - currentValue;
        effectSettings.maxValue = maxValue;

        effectBarsToRender.push_back(EffectBarToRender{ effectSettings, GetTexture(texturePath), color, decimals });
    };

    // HP, FP, Stamina, Stagger
    AddStatusBar(BarType::HP, static_cast<float>(targetChrIns->moduleBag->statModule->hp),
                 static_cast<float>(targetChrIns->moduleBag->statModule->maxHp), "Red.png");

    AddStatusBar(BarType::FP, static_cast<float>(targetChrIns->moduleBag->statModule->fp),
                 static_cast<float>(targetChrIns->moduleBag->statModule->maxFp), "Blue.png");

    if (targetChrIns->chrType != 5) {
        AddStatusBar(BarType::Stamina, static_cast<float>(targetChrIns->moduleBag->statModule->stamina),
                     static_cast<float>(targetChrIns->moduleBag->statModule->maxStamina), "Green.png");
    }

    AddStatusBar(BarType::Stagger, targetChrIns->moduleBag->superArmorModule->stagger,
                 targetChrIns->moduleBag->superArmorModule->maxStagger, "Yellow.png", 2);

    // Poison, ScarletRot, Hemorrhage, DeathBlight, Frostbite, Sleep, Madness
    AddEffectBar(BarType::Poison, static_cast<float>(targetChrIns->moduleBag->resistModule->poisonResist),
                 static_cast<float>(targetChrIns->moduleBag->resistModule->maxPoisonResist), "Poison.png",
                 IM_COL32(100, 113, 0, 255));

    AddEffectBar(BarType::ScarletRot, static_cast<float>(targetChrIns->moduleBag->resistModule->scarletRotResist),
                 static_cast<float>(targetChrIns->moduleBag->resistModule->maxScarletRotResist), "ScarletRot.png",
                 IM_COL32(117, 38, 1, 255));

    AddEffectBar(BarType::Hemorrhage, static_cast<float>(targetChrIns->moduleBag->resistModule->hemorrhageResist),
                 static_cast<float>(targetChrIns->moduleBag->resistModule->maxHemorrhageResist), "Hemorrhage.png",
                 IM_COL32(95, 11, 11, 255));

    AddEffectBar(BarType::DeathBlight, static_cast<float>(targetChrIns->moduleBag->resistModule->deathBlightResist),
                 static_cast<float>(targetChrIns->moduleBag->resistModule->maxDeathBlightResist), "DeathBlight.png",
                 IM_COL32(57, 53, 50, 255));

    AddEffectBar(BarType::Frostbite, static_cast<float>(targetChrIns->moduleBag->resistModule->frostbiteResist),
                 static_cast<float>(targetChrIns->moduleBag->resistModule->maxFrostbiteResist), "Frostbite.png",
                 IM_COL32(48, 95, 133, 255));

    AddEffectBar(BarType::Sleep, static_cast<float>(targetChrIns->moduleBag->resistModule->sleepResist),
                 static_cast<float>(targetChrIns->moduleBag->resistModule->maxSleepResist), "Sleep.png",
                 IM_COL32(77, 80, 114, 255));

    AddEffectBar(BarType::Madness, static_cast<float>(targetChrIns->moduleBag->resistModule->madnessResist),
                 static_cast<float>(targetChrIns->moduleBag->resistModule->maxMadnessResist), "Madness.png",
                 IM_COL32(131, 85, 0, 255));

    // Render Stat Bars
    for (size_t i = 0; i < barsToRender.size(); ++i) {
        BarToRender& bar = barsToRender[i];
        bar.settings.position.y += static_cast<float>(i) * (Config::defaultBarSettings.size.y - 10); // Espaçamento vertical entre barras
        barRenderer.Render(drawList, bar.settings, bar.textureInfo, bar.decimals);
    }

    const float paddingY = (Config::defaultBarSettings.size.y - 10) * static_cast<float>(barsToRender.size()) + 10.0f;
    int shownIndex = 0;

    // Render Effect Bars
    for (const auto& effectBar : effectBarsToRender) {
        if (effectBar.settings.currentValue <= 0.0f) {
            continue;
        }

        BarSettings adjustedSettings = effectBar.settings;
        adjustedSettings.position.y += (static_cast<float>(shownIndex) * (Config::effectBarIconSize.y)) + paddingY;
        shownIndex++;

        effectBarRenderer.Render(drawList, adjustedSettings, effectBar.textureInfo, effectBar.barColor, effectBar.decimals);
    }

    ImGui::EndFrame();
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
    InitializeRenderTargers(pSwapChain);

    if (!ImGui::GetCurrentContext()) {
        device->Release();
        return;
    }

    Draw(device);

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
            } else {
                Logger::Error("Texture " + fileName + " failed to load");
            }
        }
    }
}

bool Overlay::LoadTextureFromFile(const char* fileName, ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle, TextureInfo* textureInfo) {
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
    bool ret = LoadTextureFromMemory(fileData, fileSize, device, srvCpuHandle, textureInfo, Config::opacity);
    IM_FREE(fileData);

    return ret;
}

bool Overlay::LoadTextureFromMemory(const void* data, size_t data_size, ID3D12Device* d3d_device, D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle, TextureInfo* textureInfo, float opacity){
    // Load from disk into a raw RGBA buffer
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    if (opacity < 0.0f) opacity = 0.0f;
    if (opacity > 1.0f) opacity = 1.0f;
    for (int y = 0; y < image_height; y++) {
        for (int x = 0; x < image_width; x++) {
            int index = (y * image_width + x) * 4; // RGBA data
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

LRESULT WINAPI Overlay::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    }

    return CallWindowProcW(reinterpret_cast<WNDPROC>(oWndProc_), hWnd, uMsg, wParam, lParam);
}

} // souls_vision
//
// Created by PC-SAMUEL on 22/11/2024.
//

#ifndef SOULS_VISION_OVERLAY_H
#define SOULS_VISION_OVERLAY_H

#include <imgui.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <wrl/client.h>
#include "shared_types.h"
#include "stat_bar.h"
#include "structs/world_chr_man_imp.h"
#include "effect_bar.h"

using namespace structs;

namespace souls_vision {

class Overlay {
public:
    static void Initialize();
    static void Uninitialize();
    static TextureInfo GetTexture(const std::string& textureName);

    static ID3D12CommandQueue* commandQueue_;
    static ImFont* font_;

private:
    static void Render(IDXGISwapChain3* pSwapChain);
    static void CleanupRenderTargets();
    static void InitializeDXResources(IDXGISwapChain3* pSwapChain);
    static void InitializeFileResources(ID3D12Device* device);
    static void InitializeRenderTargets(IDXGISwapChain3* pSwapChain);
    static void InitializeBars(ID3D12Device* device);
    static void DrawStatBars(ID3D12Device* device);
    static void RenderTargets(IDXGISwapChain3 *pSwapChain);
    static inline int GetCorrectDXGIFormat(int eCurrentFormat);
    static void LoadAllTextures(ID3D12Device* device);
    static bool LoadTextureFromFile(const char* fileName, ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle, TextureInfo* textureInfo, bool grayscale = false);
    static bool LoadTextureFromMemory(const void* data, size_t data_size, ID3D12Device* d3d_device, D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle, TextureInfo* textureInfo, float opacity = 1.0f, bool grayscale = false);
    static int GetTextureCount();
    static std::string GetTextureNameForType(BarType type, bool grayscale = false);
    static float GetTargetValue(BarType type, ChrIns* targetChrIns);
    static float GetTargetMaxValue(BarType type, ChrIns* targetChrIns);
    static bool GetBarVisibility(BarType type);
    static ImVec4 GetColor0To1(int r, int g, int b, int a);
    static ImFont* LoadFont();

    static LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static uint64_t oWndProc_;
    static ID3D12DescriptorHeap *srvHeap_;
    static ID3D12DescriptorHeap *rtvHeap_;
    static ID3D12CommandAllocator **commandAllocators_;
    static ID3D12GraphicsCommandList *commandList_;
    static ID3D12Resource **renderTargets_;
    static std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> cpuDescriptorHandles_;
    static uint64_t buffersCounts_;

    static int textureCount_;
    static std::unordered_map<std::string, TextureInfo> textureMap_;

    static std::vector<StatBar> statBars_;
    static std::vector<EffectBar> effectBars_;
    static std::vector<BarToRender> barsToRender_;
    static std::vector<BarToRender> effectBarsToRender_;
    static std::vector<std::string> effectsNames;
};

} // souls_vision

#endif //SOULS_VISION_OVERLAY_H

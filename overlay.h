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
#include "bar_renderer.h"
#include "effect_bar_renderer.h"
#include "shared_types.h"

namespace souls_vision {

class Overlay {
public:
    static void Initialize();
    static void Uninitialize();
    static TextureInfo GetTexture(const std::string& textureName);

    static ID3D12CommandQueue* commandQueue_;

private:
    static void Render(IDXGISwapChain3* pSwapChain);
    static void CleanupRenderTargets();
    static void InitializeDXResources(IDXGISwapChain3* pSwapChain);
    static void InitializeRenderTargers(IDXGISwapChain3* pSwapChain);
    static void InitializeBars(ID3D12Device* device);
    static void Draw(ID3D12Device* device);
    static void RenderTargets(IDXGISwapChain3 *pSwapChain);
    static inline int GetCorrectDXGIFormat(int eCurrentFormat);
    static void LoadAllTextures(ID3D12Device* device);
    static bool LoadTextureFromFile(const char* fileName, ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle, TextureInfo* textureInfo);
    static bool LoadTextureFromMemory(const void* data, size_t data_size, ID3D12Device* d3d_device, D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle, TextureInfo* textureInfo, float opacity = 1.0f);
    static int GetTextureCount();
    static std::string GetTextureNameForType(BarType type);
    static void AddStatBar(BarConfig barConfig);
    static void AddEffectBar(BarConfig barConfig);

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

    static BarRenderer* barRenderer_;
    static EffectBarRenderer* effectBarRenderer_;
    static std::vector<BarToRender> barsToRender_;
    static std::vector<BarToRender> effectBarsToRender_;
};

} // souls_vision

#endif //SOULS_VISION_OVERLAY_H

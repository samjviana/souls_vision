//
// Created by PC-SAMUEL on 22/11/2024.
//

#ifndef SOULS_VISION_HOOK_HELPER_H
#define SOULS_VISION_HOOK_HELPER_H

#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include <memory>
#include <functional>

namespace souls_vision {

class HookHelper {
public:
    HookHelper() = default;
    ~HookHelper() = default;

    static void Hook();
    static void Unhook();

    static void SetRenderCallback(std::function<void(IDXGISwapChain3*)> callback);
    static void SetCleanupCallback(std::function<void()> callback);
    static void SetCommandQueue(ID3D12CommandQueue* pCommandQueue);

    // These needs to be public for the hooks to work
    static void *fnPresent_;
    static void *fnPresent1_;
    static void *fnResizeBuffers_;
    static void *fnSetSourceSize_;
    static void *fnResizeBuffers1_;
    static void *fnExecuteCommandLists_;
    static void *fnCreateSwapChain_;
    static void *fnCreateSwapChainForHwndChain_;
    static void *fnCreateSwapChainForCWindowChain_;
    static void *fnCreateSwapChainForCompChain_;

private:
    static void InitHooks();
    static bool CreateDevice();
    static void CallRenderCallback(IDXGISwapChain3* pSwapChain);
    static void CallCleanupCallback();
    static void UpdateCommandQueue(ID3D12CommandQueue* pCommandQueue);

    static std::function<void(IDXGISwapChain3*)> renderCallback_;
    static std::function<void()> cleanupCallback_;
    static ID3D12CommandQueue* pCommandQueue_;

    static HRESULT WINAPI hkPresent(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT Flags);
    static HRESULT WINAPI hkPresent1(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS *pPresentParameters);
    static HRESULT WINAPI hkResizeBuffers(IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    static HRESULT WINAPI hkSetSourceSize(IDXGISwapChain1 *pSwapChain, UINT Width, UINT Height);
    static HRESULT WINAPI hkResizeBuffers1(IDXGISwapChain3 *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags, const UINT *pCreationNodeMask, IUnknown *const *ppPresentQueue);
    static void WINAPI hkExecuteCommandLists(ID3D12CommandQueue *pCommandQueue, UINT NumCommandLists, ID3D12CommandList *ppCommandLists);
    static HRESULT WINAPI hkCreateSwapChain(IDXGIFactory *pFactory, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain);
    static HRESULT WINAPI hkCreateSwapChainForHwnd(IDXGIFactory *pFactory, IUnknown *pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1 *pDesc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc, IDXGIOutput *pRestrictToOutput, IDXGISwapChain1 **ppSwapChain);
    static HRESULT WINAPI hkCreateSwapChainForCoreWindow(IDXGIFactory *pFactory, IUnknown *pDevice, IUnknown *pWindow, const DXGI_SWAP_CHAIN_DESC1 *pDesc, IDXGIOutput *pRestrictToOutput, IDXGISwapChain1 **ppSwapChain);
    static HRESULT WINAPI hkCreateSwapChainForComposition(IDXGIFactory *pFactory, IUnknown *pDevice, const DXGI_SWAP_CHAIN_DESC1 *pDesc, IDXGIOutput *pRestrictToOutput, IDXGISwapChain1 **ppSwapChain);

    static std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain3*, UINT, UINT)> oPresent_;
    static std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain3*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*)> oPresent1_;
    static std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)> oResizeBuffers_;
    static std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain1*, UINT, UINT)> oSetSourceSize_;
    static std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain3*, UINT, UINT, UINT, DXGI_FORMAT, UINT, const UINT*, IUnknown* const*)> oResizeBuffers1_;
    static std::add_pointer_t<void WINAPI(ID3D12CommandQueue*, UINT, ID3D12CommandList*)> oExecuteCommandLists_;
    static std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**)> oCreateSwapChain_;
    static std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, HWND, const DXGI_SWAP_CHAIN_DESC1*, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC*, IDXGIOutput*, IDXGISwapChain1**)> oCreateSwapChainForHwndChain_;
    static std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**)> oCreateSwapChainForCWindowChain_;
    static std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**)> oCreateSwapChainForCompChain_;
};

} // souls_vision

#endif //SOULS_VISION_HOOK_HELPER_H

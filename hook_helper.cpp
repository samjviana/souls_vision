//
// Created by PC-SAMUEL on 22/11/2024.
//
#define NOMINMAX
#include "hook_helper.h"
#include "overlay.h"
#include "logger.h"

#include <wrl/client.h>
#include <MinHook.h>
#include <iostream>
#include <windows.h>

namespace souls_vision {

std::function<void(IDXGISwapChain3*)> HookHelper::renderCallback_ = nullptr;
std::function<void()> HookHelper::cleanupCallback_ = nullptr;
ID3D12CommandQueue* HookHelper::pCommandQueue_ = nullptr;

std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain3*, UINT, UINT)> HookHelper::oPresent_;
std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain3*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*)> HookHelper::oPresent1_;
std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)> HookHelper::oResizeBuffers_;
std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain1*, UINT, UINT)> HookHelper::oSetSourceSize_;
std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain3*, UINT, UINT, UINT, DXGI_FORMAT, UINT, const UINT*, IUnknown* const*)> HookHelper::oResizeBuffers1_;
std::add_pointer_t<void WINAPI(ID3D12CommandQueue*, UINT, ID3D12CommandList*)> HookHelper::oExecuteCommandLists_;
std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**)> HookHelper::oCreateSwapChain_;
std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, HWND, const DXGI_SWAP_CHAIN_DESC1*, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC*, IDXGIOutput*, IDXGISwapChain1**)> HookHelper::oCreateSwapChainForHwndChain_;
std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**)> HookHelper::oCreateSwapChainForCWindowChain_;
std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**)> HookHelper::oCreateSwapChainForCompChain_;

void* HookHelper::fnPresent_ = nullptr;
void* HookHelper::fnPresent1_ = nullptr;
void* HookHelper::fnResizeBuffers_ = nullptr;
void* HookHelper::fnSetSourceSize_ = nullptr;
void* HookHelper::fnResizeBuffers1_ = nullptr;
void* HookHelper::fnExecuteCommandLists_ = nullptr;
void* HookHelper::fnCreateSwapChain_ = nullptr;
void* HookHelper::fnCreateSwapChainForHwndChain_ = nullptr;
void* HookHelper::fnCreateSwapChainForCWindowChain_ = nullptr;
void* HookHelper::fnCreateSwapChainForCompChain_ = nullptr;

void HookHelper::Hook() {
    MH_Initialize();
    InitHooks();

    Logger::Info("Hooks initialized successfully.");
}

void HookHelper::Unhook() {
    MH_DisableHook(fnPresent_);
    MH_DisableHook(fnPresent1_);
    MH_DisableHook(fnResizeBuffers_);
    MH_DisableHook(fnSetSourceSize_);
    MH_DisableHook(fnResizeBuffers1_);
    MH_DisableHook(fnExecuteCommandLists_);
    MH_DisableHook(fnCreateSwapChain_);
    MH_DisableHook(fnCreateSwapChainForHwndChain_);
    MH_DisableHook(fnCreateSwapChainForCWindowChain_);
    MH_DisableHook(fnCreateSwapChainForCompChain_);

    MH_Uninitialize();

    Logger::Info("Hooks unhooked successfully.");
}

void HookHelper::SetRenderCallback(std::function<void(IDXGISwapChain3 *)> callback) {
    renderCallback_ = std::move(callback);
}

void HookHelper::SetCleanupCallback(std::function<void()> callback) {
    cleanupCallback_ = std::move(callback);
}

void HookHelper::SetCommandQueue(ID3D12CommandQueue *pCommandQueue) {
    pCommandQueue_ = pCommandQueue;
}

void HookHelper::InitHooks() {
    if (!HookHelper::CreateDevice()) {
        Logger::Error("Failed to create device");
        return;
    }

    struct HookInfo {
        void* targetFunction;
        void* hookFunction;
        void** originalFunction;
        const char* hookName;
    };

    std::vector<HookInfo> hooks = {
        { fnPresent_, reinterpret_cast<void*>(&hkPresent), reinterpret_cast<void**>(&oPresent_), "Present" },
        { fnPresent1_, reinterpret_cast<void*>(&hkPresent1), reinterpret_cast<void**>(&oPresent1_), "Present1" },
        { fnResizeBuffers_, reinterpret_cast<void*>(&hkResizeBuffers), reinterpret_cast<void**>(&oResizeBuffers_), "ResizeBuffers" },
        { fnSetSourceSize_, reinterpret_cast<void*>(&hkSetSourceSize), reinterpret_cast<void**>(&oSetSourceSize_), "SetSourceSize" },
        { fnResizeBuffers1_, reinterpret_cast<void*>(&hkResizeBuffers1), reinterpret_cast<void**>(&oResizeBuffers1_), "ResizeBuffers1" },
        { fnExecuteCommandLists_, reinterpret_cast<void*>(&hkExecuteCommandLists), reinterpret_cast<void**>(&oExecuteCommandLists_), "ExecuteCommandLists" },
        { fnCreateSwapChain_, reinterpret_cast<void*>(&hkCreateSwapChain), reinterpret_cast<void**>(&oCreateSwapChain_), "CreateSwapChain" },
        { fnCreateSwapChainForHwndChain_, reinterpret_cast<void*>(&hkCreateSwapChainForHwnd), reinterpret_cast<void**>(&oCreateSwapChainForHwndChain_), "CreateSwapChainForHwnd" },
        { fnCreateSwapChainForCWindowChain_, reinterpret_cast<void*>(&hkCreateSwapChainForCoreWindow), reinterpret_cast<void**>(&oCreateSwapChainForCWindowChain_), "CreateSwapChainForCoreWindow" },
        { fnCreateSwapChainForCompChain_, reinterpret_cast<void*>(&hkCreateSwapChainForComposition), reinterpret_cast<void**>(&oCreateSwapChainForCompChain_), "CreateSwapChainForComposition" }
    };

    for (const auto& hook : hooks) {
        MH_STATUS status = MH_CreateHook(hook.targetFunction, hook.hookFunction, hook.originalFunction);
        if (status != MH_OK) {
            Logger::Error("Failed to create hook for " + std::string(hook.hookName) + ": " + MH_StatusToString(status));
        }
    }

    // The order of enabling the hooks is important
    MH_EnableHook(fnCreateSwapChain_);
    MH_EnableHook(fnCreateSwapChainForHwndChain_);
    MH_EnableHook(fnCreateSwapChainForCWindowChain_);
    MH_EnableHook(fnCreateSwapChainForCompChain_);
    MH_EnableHook(fnPresent_);
    MH_EnableHook(fnPresent1_);
    MH_EnableHook(fnResizeBuffers_);
    MH_EnableHook(fnSetSourceSize_);
    MH_EnableHook(fnResizeBuffers1_);
    MH_EnableHook(fnExecuteCommandLists_);
}

bool HookHelper::CreateDevice() {
    const wchar_t CLASS_NAME[] = L"OverlayWindowClass";
    const wchar_t WINDOW_NAME[] = L"OverlayWindow";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        Logger::Error("Failed to register window class. Error: " + std::to_string(GetLastError()));
        return false;
    }

    HWND hwnd = CreateWindowExW(
            0,
            CLASS_NAME,
            WINDOW_NAME,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 100, 100,
            nullptr,
            nullptr,
            wc.hInstance,
            nullptr
    );

    if (!hwnd) {
        Logger::Error("Failed to create window. Error: " + std::to_string(GetLastError()));
        UnregisterClassW(CLASS_NAME, wc.hInstance);
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr)) {
        Logger::Error("Failed to create DXGI factory. HRESULT: " + std::to_string(hr));
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    hr = dxgiFactory->EnumAdapters1(0, &adapter);
    if (FAILED(hr)) {
        Logger::Error("Failed to enumerate adapters. HRESULT: " + std::to_string(hr));
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
    hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice));
    if (FAILED(hr)) {
        Logger::Error("Failed to create D3D12 device. HRESULT: " + std::to_string(hr));
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.NodeMask = 0;

    hr = d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    if (FAILED(hr)) {
        Logger::Error("Failed to create command queue. HRESULT: " + std::to_string(hr));
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = 100;
    swapChainDesc.Height = 100;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    hr = dxgiFactory->CreateSwapChainForHwnd(
            commandQueue.Get(),
            hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain1
    );

    if (FAILED(hr)) {
        Logger::Error("Failed to create swap chain. HRESULT: " + std::to_string(hr));
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;
    hr = swapChain1.As(&swapChain3);
    if (FAILED(hr)) {
        Logger::Error("QueryInterface for IDXGISwapChain3 failed. HRESULT: " + std::to_string(hr));
        return false;
    }

    void** swapChainVTable = *reinterpret_cast<void***>(swapChain3.Get());
    void** commandQueueVTable = *reinterpret_cast<void***>(commandQueue.Get());
    void** dxgiFactoryVTable = *reinterpret_cast<void***>(dxgiFactory.Get());

    fnPresent_ = swapChainVTable[8];
    fnPresent1_ = swapChainVTable[22];
    fnResizeBuffers_ = swapChainVTable[13];
    fnSetSourceSize_ = swapChainVTable[29];
    fnResizeBuffers1_ = swapChainVTable[39];
    fnExecuteCommandLists_ = commandQueueVTable[10];
    fnCreateSwapChain_ = dxgiFactoryVTable[10];
    fnCreateSwapChainForHwndChain_ = dxgiFactoryVTable[15];
    fnCreateSwapChainForCWindowChain_ = dxgiFactoryVTable[16];
    fnCreateSwapChainForCompChain_ = dxgiFactoryVTable[24];

    Logger::Info("Device created successfully.");

    return true;
}

void HookHelper::CallRenderCallback(IDXGISwapChain3 *pSwapChain) {
    if (renderCallback_) {
        renderCallback_(pSwapChain);
    }
}

void HookHelper::CallCleanupCallback() {
    if (cleanupCallback_) {
        cleanupCallback_();
    }
}

void HookHelper::UpdateCommandQueue(ID3D12CommandQueue *pCommandQueue) {
    if (!Overlay::commandQueue_) {
        Overlay::commandQueue_ = pCommandQueue;
    }
}


HRESULT WINAPI HookHelper::hkPresent(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT Flags) {
    CallRenderCallback(pSwapChain);
    return oPresent_(pSwapChain, SyncInterval, Flags);
}

HRESULT WINAPI HookHelper::hkPresent1(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS *pPresentParameters) {
    CallRenderCallback(pSwapChain);
    return oPresent1_(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

HRESULT WINAPI HookHelper::hkResizeBuffers(IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    CallCleanupCallback();
    return oResizeBuffers_(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

HRESULT WINAPI HookHelper::hkSetSourceSize(IDXGISwapChain1 *pSwapChain, UINT Width, UINT Height) {
    CallCleanupCallback();
    return oSetSourceSize_(pSwapChain, Width, Height);
}

HRESULT WINAPI HookHelper::hkResizeBuffers1(IDXGISwapChain3 *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags, const UINT *pCreationNodeMask, IUnknown *const *ppPresentQueue) {
    CallCleanupCallback();
    return oResizeBuffers1_(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
}

void WINAPI HookHelper::hkExecuteCommandLists(ID3D12CommandQueue *pCommandQueue, UINT NumCommandLists, ID3D12CommandList *ppCommandLists) {
    UpdateCommandQueue(pCommandQueue);
    return oExecuteCommandLists_(pCommandQueue, NumCommandLists, ppCommandLists);
}

HRESULT WINAPI HookHelper::hkCreateSwapChain(IDXGIFactory *pFactory, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain) {
    CallCleanupCallback();
    return oCreateSwapChain_(pFactory, pDevice, pDesc, ppSwapChain);
}

HRESULT WINAPI HookHelper::hkCreateSwapChainForHwnd(IDXGIFactory *pFactory, IUnknown *pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1 *pDesc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc, IDXGIOutput *pRestrictToOutput, IDXGISwapChain1 **ppSwapChain) {
    CallCleanupCallback();
    return oCreateSwapChainForHwndChain_(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
}

HRESULT WINAPI HookHelper::hkCreateSwapChainForCoreWindow(IDXGIFactory *pFactory, IUnknown *pDevice, IUnknown *pWindow, const DXGI_SWAP_CHAIN_DESC1 *pDesc, IDXGIOutput *pRestrictToOutput, IDXGISwapChain1 **ppSwapChain) {
    CallCleanupCallback();
    return oCreateSwapChainForCWindowChain_(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
}

HRESULT WINAPI HookHelper::hkCreateSwapChainForComposition(IDXGIFactory *pFactory, IUnknown *pDevice, const DXGI_SWAP_CHAIN_DESC1 *pDesc, IDXGIOutput *pRestrictToOutput, IDXGISwapChain1 **ppSwapChain) {
    CallCleanupCallback();
    return oCreateSwapChainForCompChain_(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);
}

} // souls_vision
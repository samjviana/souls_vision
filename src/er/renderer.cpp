#include "renderer.h"
#include "hook_helper.h"
#include "overlay.h"

namespace souls_vision {

void Renderer::Initialize() {
    HookHelper::SetRenderCallback(Overlay::Render);
    HookHelper::SetCleanupCallback(Overlay::CleanupRenderTargets);
    Overlay::Initialize();
}

void Renderer::Uninitialize() {
    Overlay::Uninitialize();
}

} // namespace souls_vision

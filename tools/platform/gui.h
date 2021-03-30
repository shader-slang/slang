// gui.h
#pragma once

#include "slang-gfx.h"
#include "vector-math.h"
#include "window.h"
#include "slang-com-ptr.h"
#include "external/imgui/imgui.h"
#include "source/core/slang-basic.h"

namespace platform {

struct GUI : Slang::RefObject
{
    GUI(Window* window,
        gfx::IDevice* device,
        gfx::ICommandQueue* queue,
        gfx::IFramebufferLayout* framebufferLayout);
    ~GUI();

    void beginFrame();
    void endFrame(gfx::ITransientResourceHeap* transientHeap, gfx::IFramebuffer* framebuffer);

private:
    Slang::ComPtr<gfx::IDevice> device;
    Slang::ComPtr<gfx::ICommandQueue> queue;
    Slang::ComPtr<gfx::IRenderPassLayout> renderPass;
    Slang::ComPtr<gfx::IPipelineState>       pipelineState;
    Slang::ComPtr<gfx::ISamplerState>        samplerState;
};

} // gfx

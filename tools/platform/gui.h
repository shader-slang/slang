// gui.h
#pragma once

#include "core/slang-basic.h"
#include "imgui/imgui.h"
#include "slang-com-ptr.h"
#include "vector-math.h"
#include "window.h"

#include <slang-rhi.h>

namespace platform
{

struct GUI : Slang::RefObject
{
    GUI(Window* window, rhi::IDevice* device, rhi::ICommandQueue* queue);
    ~GUI();

    void beginFrame();
    void endFrame(rhi::ITexture* renderTarget);

private:
    Slang::ComPtr<rhi::IDevice> device;
    Slang::ComPtr<rhi::ICommandQueue> queue;
    Slang::ComPtr<rhi::IRenderPipeline> pipelineState;
    Slang::ComPtr<rhi::ISampler> samplerState;
    Slang::ComPtr<rhi::IShaderProgram> shaderProgram;
    Slang::ComPtr<rhi::IInputLayout> inputLayout;
};

} // namespace platform

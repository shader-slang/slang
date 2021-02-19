// gui.h
#pragma once

#include "slang-gfx.h"
#include "vector-math.h"
#include "window.h"
#include "slang-com-ptr.h"
#include "external/imgui/imgui.h"
#include "source/core/slang-basic.h"

namespace gfx {

struct GUI : Slang::RefObject
{
    GUI(Window* window, IRenderer* renderer);
    ~GUI();

    void beginFrame();
    void endFrame();

private:
    Slang::ComPtr<IRenderer>    renderer;
    Slang::ComPtr<IPipelineState>       pipelineState;
    Slang::ComPtr<IDescriptorSetLayout> descriptorSetLayout;
    Slang::ComPtr<IPipelineLayout>      pipelineLayout;
    Slang::ComPtr<ISamplerState>        samplerState;
};

} // gfx

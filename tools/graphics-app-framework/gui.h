// gui.h
#pragma once

#include "tools/gfx/render.h"
#include "vector-math.h"
#include "window.h"
#include "slang-com-ptr.h"
#include "external/imgui/imgui.h"

namespace gfx {

struct GUI : RefObject
{
    GUI(Window* window, IRenderer* renderer);
    ~GUI();

    void beginFrame();
    void endFrame();

private:
    Slang::ComPtr<IRenderer>    renderer;
    RefPtr<PipelineState>       pipelineState;
    RefPtr<DescriptorSetLayout> descriptorSetLayout;
    RefPtr<PipelineLayout>      pipelineLayout;
    RefPtr<SamplerState>        samplerState;
};

} // gfx

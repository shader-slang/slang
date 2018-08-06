// gui.h
#pragma once

#include "render.h"
#include "vector-math.h"
#include "window.h"

#include "external/imgui/imgui.h"

namespace gfx {

struct GUI : RefObject
{
    GUI(Window* window, Renderer* renderer);
    ~GUI();

    void beginFrame();
    void endFrame();

private:
    RefPtr<Renderer>            renderer;
    RefPtr<PipelineState>       pipelineState;
    RefPtr<DescriptorSetLayout> descriptorSetLayout;
    RefPtr<PipelineLayout>      pipelineLayout;
    RefPtr<SamplerState>        samplerState;
};

} // gfx

// render-d3d12.h
#pragma once

#include <cstdint>

namespace gfx {

class IRenderer;

int32_t createD3D12Renderer(IRenderer** outRenderer);

} // gfx

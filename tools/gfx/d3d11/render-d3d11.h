// render-d3d11.h
#pragma once

#include <cstdint>

namespace gfx {

class IRenderer;

int32_t createD3D11Renderer(IRenderer** outRenderer);

} // gfx

// render-d3d11.h
#pragma once

#include <cstdint>

namespace gfx {

class IRenderer;

int32_t createGLRenderer(IRenderer** outRenderer);

} // gfx

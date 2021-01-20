// render-d3d11.h
#pragma once

#include <cstdint>
#include "slang.h"

namespace gfx {

class IRenderer;

SlangResult SLANG_MCALL createD3D11Renderer(IRenderer** outRenderer);

} // gfx

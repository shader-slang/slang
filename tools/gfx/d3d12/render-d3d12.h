// render-d3d12.h
#pragma once

#include <cstdint>
#include "slang.h"

namespace gfx {

class IRenderer;

SlangResult SLANG_MCALL createD3D12Renderer(IRenderer** outRenderer);

} // gfx

// render-d3d12.h
#pragma once

#include "../renderer-shared.h"

namespace gfx
{

SlangResult SLANG_MCALL createD3D12Renderer(const IRenderer::Desc* desc, void* windowHandle, IRenderer** outRenderer);

} // gfx

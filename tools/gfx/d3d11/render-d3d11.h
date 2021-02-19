// render-d3d11.h
#pragma once

#include "../renderer-shared.h"

namespace gfx
{

SlangResult SLANG_MCALL createD3D11Renderer(const IRenderer::Desc* desc, IRenderer** outRenderer);

} // gfx

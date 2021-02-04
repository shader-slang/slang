// render-d3d11.h
#pragma once

#include "../renderer-shared.h"

namespace gfx {

SlangResult SLANG_MCALL createGLRenderer(const IRenderer::Desc* desc, void* windowHandle, IRenderer** outRenderer);

} // gfx

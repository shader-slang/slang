#pragma once

#include "../renderer-shared.h"

namespace gfx
{

SlangResult SLANG_MCALL createCUDARenderer(const IRenderer::Desc* desc, void* windowHandle, IRenderer** outRenderer);
}

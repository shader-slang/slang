// render-vk.h
#pragma once

#include <cstdint>
#include "../renderer-shared.h"

namespace gfx {

SlangResult SLANG_MCALL createVKRenderer(const IRenderer::Desc* desc, IRenderer** outRenderer);

} // gfx

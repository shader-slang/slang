// render-vk.h
#pragma once

#include <cstdint>
#include "slang.h"

namespace gfx {

class IRenderer;

SlangResult SLANG_MCALL createVKRenderer(IRenderer** outRenderer);

} // gfx

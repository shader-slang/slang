#pragma once

#include <cstdint>

namespace gfx
{
class IRenderer;

int32_t createCUDARenderer(IRenderer** outRenderer);
}

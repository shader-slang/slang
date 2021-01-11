// render-vk.h
#pragma once

#include <cstdint>

namespace gfx {

class IRenderer;

int32_t createVKRenderer(IRenderer** outRenderer);

} // gfx

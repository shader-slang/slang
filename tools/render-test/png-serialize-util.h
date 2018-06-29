// png-serialize-util.h
#pragma once

#include "surface.h"

namespace renderer_test {

using namespace slang_graphics;

struct PngSerializeUtil
{
    static Slang::Result write(const char* filename, const Surface& surface);

};

} // renderer_test

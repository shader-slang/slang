// png-serialize-util.cpp
#define _CRT_SECURE_NO_WARNINGS

#include "png-serialize-util.h"

#include <stdlib.h>
#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"

namespace renderer_test {
using namespace Slang;

/* static */Slang::Result PngSerializeUtil::write(const char* filename, const Surface& surface)
{
    int numComps = 0;
    switch (surface.m_format)
    {
        case Format::RGBA_Unorm_UInt8:
        {
            numComps = 4;
            break;
        }
        default: break;
    }

    if (numComps <= 0)
    {
        return SLANG_FAIL;
    }

    int stbResult = stbi_write_png(filename, surface.m_width, surface.m_height, numComps, surface.m_data, surface.m_rowStrideInBytes);

    return stbResult ? SLANG_OK : SLANG_FAIL;
}

} // renderer_test

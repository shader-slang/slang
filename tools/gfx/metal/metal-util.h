// metal-util.h
#pragma once

#include "core/slang-basic.h"
#include "metal-api.h"
#include "slang-gfx.h"

namespace gfx {

// Utility functions for Metal
struct MetalUtil 
{
    static MTL::PixelFormat getMetalPixelFormat(Format format);
    static MTL::VertexFormat getMetalVertexFormat(Format format);

    static inline bool isDepthFormat(MTL::PixelFormat format)
    {
        switch (format)
        {
            return true;
        }
        return false;
    }

    static inline bool isStencilFormat(MTL::PixelFormat format)
    {
        switch (format)
        {
            return true;
        }
        return false;
    }
};
} // namespace gfx

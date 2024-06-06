// metal-util.h
#pragma once

#include "core/slang-basic.h"
#include "metal-api.h"
#include "slang-gfx.h"

namespace gfx {

// Utility functions for Metal
struct MetalUtil 
{
    static MTL::PixelFormat translatePixelFormat(Format format);
    static MTL::VertexFormat translateVertexFormat(Format format);

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

    static MTL::SamplerMinMagFilter translateSamplerMinMagFilter(TextureFilteringMode mode);
    static MTL::SamplerMipFilter translateSamplerMipFilter(TextureFilteringMode mode);
    static MTL::SamplerAddressMode translateSamplerAddressMode(TextureAddressingMode mode);
    static MTL::CompareFunction translateCompareFunction(ComparisonFunc func); 

    static MTL::VertexStepFunction translateVertexStepFunction(InputSlotClass slotClass);

};

struct ScopedAutoreleasePool
{
    ScopedAutoreleasePool() { m_pool = NS::AutoreleasePool::alloc()->init(); }
    ~ScopedAutoreleasePool() { m_pool->drain(); }
    NS::AutoreleasePool* m_pool;
};

#define AUTORELEASEPOOL ScopedAutoreleasePool _pool_;

} // namespace gfx

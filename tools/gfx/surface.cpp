// surface.cpp
#include "surface.h"

#include <stdlib.h>
#include <stdio.h>

#include "../../source/core/slang-list.h"

namespace gfx {
using namespace Slang;

class MallocSurfaceAllocator: public SurfaceAllocator
{
    public:

    virtual Slang::Result allocate(int width, int height, Format format, int alignment, Surface& surface) override;
    virtual void deallocate(Surface& surface) override;
};

static MallocSurfaceAllocator s_mallocSurfaceAllocator;

/// Get the malloc allocator
/* static */SurfaceAllocator* SurfaceAllocator::getMallocAllocator()
{
    return &s_mallocSurfaceAllocator;
}

Slang::Result MallocSurfaceAllocator::allocate(int width, int height, Format format, int alignment, Surface& surface)
{
    assert(surface.m_data == nullptr);

    // Calculate row size

    const int rowSizeInBytes = Surface::calcRowSize(format, width);
    const int numRows = Surface::calcNumRows(format, height);

    alignment = (alignment <= 0) ? int(sizeof(void*)) : alignment;
    // It must be a power of 2
    assert( ((alignment - 1)  & alignment) == 0);

    // Align rowSize
    const int alignedRowSizeInBytes = (rowSizeInBytes + alignment - 1) & -alignment;

    size_t totalSize = numRows * alignedRowSizeInBytes;

    uint8_t* data = (uint8_t*)::malloc(totalSize);
    if (!data)
    {
        return SLANG_E_OUT_OF_MEMORY;
    }

    surface.m_data = data;
    surface.m_width = width;
    surface.m_height = height;
    surface.m_format = format;
    surface.m_numRows = numRows;
    surface.m_rowStrideInBytes = alignedRowSizeInBytes;

    surface.m_allocator = this;
    return SLANG_OK;
}

void MallocSurfaceAllocator::deallocate(Surface& surface)
{
    assert(surface.m_data);
    // Make sure it's not an inverted, cos otherwise m_data is not the start address
    assert(surface.m_rowStrideInBytes > 0);
    ::free(surface.m_data);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Surface !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */int Surface::calcRowSize(Format format, int width)
{
    size_t pixelSize = RendererUtil::getFormatSize(format);
    if (pixelSize == 0)
    {
        return 0;
    }
    return int(pixelSize * width);
}

/* static */int Surface::calcNumRows(Format format, int height)
{
    // Don't have any compressed types, so number of rows is same as the height
    return height;
}

void Surface::init()
{
    m_width = 0;
    m_height = 0;
    m_format = Format::Unknown;
    m_data = nullptr;
    m_numRows = 0;
    m_rowStrideInBytes = 0;
    // NOTE! does not clear the allocator.
    // If called with an allocation memory will leak!
}

Surface::~Surface()
{
    if (m_data && m_allocator)
    {
        m_allocator->deallocate(*this);
    }
}

void Surface::deallocate()
{
    if (m_data && m_allocator)
    {
        m_allocator->deallocate(*this);
        init();
    }
}

Result Surface::allocate(int width, int height, Format format, int alignment, SurfaceAllocator* allocator)
{
    deallocate();
    allocator = allocator ? allocator : m_allocator;
    if (!allocator)
    {
        // An allocator needs to be set on the surface, or one passed in.
        return SLANG_FAIL;
    }
    return allocator->allocate(width, height, format, alignment, *this);
}

void Surface::setUnowned(int width, int height, Format format, int strideInBytes, void* data)
{
    deallocate();

    // This is unowned
    m_allocator = nullptr;

    m_width = width;
    m_height = height;
    m_format = format;
    m_rowStrideInBytes = strideInBytes;
    m_data = (uint8_t*)data;

    m_numRows = Surface::calcNumRows(format, height);

    const int rowSizeInBytes = Surface::calcRowSize(format, width);
    assert((strideInBytes > 0 && rowSizeInBytes <= strideInBytes) || (strideInBytes < 0 && rowSizeInBytes <= -strideInBytes));
}

void Surface::zeroContents()
{
    const int rowSizeInBytes = Surface::calcRowSize(m_format, m_width);

    const int stride = m_rowStrideInBytes;
    uint8_t* dst = m_data;

    for (int i = 0; i < m_numRows; i++, dst += stride)
    {
        ::memset(dst, 0, rowSizeInBytes);
    }
}

void Surface::flipInplaceVertically()
{
    // Can only flip when m_height matches number of rows
    assert(m_numRows == m_height);

    const int rowSizeInBytes = Surface::calcRowSize(m_format, m_width);
    if (rowSizeInBytes <= 0 || m_numRows <= 1)
    {
        return;
    }

    uint8_t* top = m_data;
    uint8_t* bottom = m_data + (m_numRows - 1) * m_rowStrideInBytes;

    List<uint8_t> bufferList;
    bufferList.setCount(rowSizeInBytes);
    uint8_t* buffer = bufferList.getBuffer();

    const int stride = m_rowStrideInBytes;

    const int num = m_height >> 1;
    for (int i = 0; i < num; ++i, top += stride, bottom -= stride)
    {
        ::memcpy(buffer, top, rowSizeInBytes);
        ::memcpy(top, bottom, rowSizeInBytes);
        ::memcpy(bottom, buffer, rowSizeInBytes);
    }
}

SlangResult Surface::set(int width, int height, Format format, int srcRowStride, const void* data, SurfaceAllocator* allocator)
{
    if (hasContents() && m_width == width && m_height == height && m_format == format)
    {
        // I can just overwrite the contents that is there
    }
    else
    {
        SLANG_RETURN_ON_FAIL(allocate(width, height, format, 0, allocator));
    }

    // Okay just need to set the contents

    {
        const size_t rowSize = calcRowSize(format, width);

        const uint8_t* srcRow = (const uint8_t*)data;
        uint8_t* dstRow = (uint8_t*)m_data;

        for (int i = 0; i < m_numRows; i++)
        {
            ::memcpy(dstRow, srcRow, rowSize);

            srcRow += srcRowStride;
            dstRow += m_rowStrideInBytes;
        }
    }

    return SLANG_OK;
}

} // renderer_test

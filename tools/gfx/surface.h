// surface.h
#pragma once

#include "render.h"

namespace gfx {

class Surface;

class SurfaceAllocator
{
    public:
    virtual Slang::Result allocate(int width, int height, Format format, int alignment, Surface& surface) = 0;
    virtual void deallocate(Surface& surface) = 0;

        /// Get the malloc allocator
    static SurfaceAllocator* getMallocAllocator();
};

class Surface
{
    public:

    enum
    {
        kDefaultAlignment = sizeof(void*)
    };

        /// Allocate
    Slang::Result allocate(int width, int height, Format format, int alignment = kDefaultAlignment, SurfaceAllocator* allocator = nullptr);

        /// Deallocate contents
    void deallocate();
        /// Initialize contents (zero sized, no data). Note that the allocator pointer is left as is
    void init();

        /// Set unowned
    void setUnowned(int width, int height, Format format, int strideInBytes, void* data);

        /// Set the contents - the memory will be owned by this surface (ie will be freed by the allocator when goes out of scope or is deallocated)
    Slang::Result set(int width, int height, Format format, int strideInBytes, const void* data, SurfaceAllocator* allocator);

    template <typename T>
    T* calcNextRow(T* ptr) const { return (T*)calcNextRow((void*)ptr); }
    template <typename T>
    const T* calcNextRow(const T* ptr) const { return (const T*)calcNextRow((const void*)ptr); }

    void* calcNextRow(void* ptr) const { return (void*)(((uint8_t*)ptr) + m_rowStrideInBytes); }
    const void* calcNextRow(const void* ptr) const { return (const void*)(((const uint8_t*)ptr) + m_rowStrideInBytes); }

        /// Writes zero to all of the contents
    void zeroContents();

        /// Flips the contents vertically in place
    void flipInplaceVertically();

        /// True if has some contents
    bool hasContents() const { return m_data != nullptr; }

        /// Ctor
    Surface() :
        m_allocator(nullptr)
    {
        init();
    }
        /// Dtor
    ~Surface();

        /// Get the size of the row in bytes
    static int calcRowSize(Format format, int width);
        /// Calculates the number of rows
    static int calcNumRows(Format format, int height);

    int m_width;
    int m_height;
    Format m_format;

    uint8_t* m_data;                    /// The data that makes up the image. If nullptr, has no data. Pointer to first 'row' of the image.

    int m_numRows;                      ///< Total amount of rows (typically same as height, but in compressed formats may be less)
    int m_rowStrideInBytes;             ///< The number of bytes between rows

    SurfaceAllocator* m_allocator;      ///< Can be null if so contents is 'unowned', if set
};

} // renderer_test

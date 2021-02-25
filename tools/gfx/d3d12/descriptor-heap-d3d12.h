#pragma once


#include <dxgi.h>
#include <d3d12.h>

#include "slang-com-ptr.h"
#include "core/slang-list.h"
#include "core/slang-virtual-object-pool.h"

namespace gfx {

/*! \brief A simple class to manage an underlying Dx12 Descriptor Heap. Allocations are made linearly in order. It is not possible to free
individual allocations, but all allocations can be deallocated with 'deallocateAll'. */
class D3D12DescriptorHeap
{
    public:
    typedef D3D12DescriptorHeap ThisType;

        /// Initialize
    Slang::Result init(ID3D12Device* device, int size, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags);
        /// Initialize with an array of handles copying over the representation
    Slang::Result init(ID3D12Device* device, const D3D12_CPU_DESCRIPTOR_HANDLE* handles, int numHandles, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

        /// Returns the number of slots that have been used
    SLANG_FORCE_INLINE int getUsedSize() const { return m_currentIndex; }

        /// Get the total amount of descriptors possible on the heap
    SLANG_FORCE_INLINE int getTotalSize() const { return m_totalSize; }
        /// Allocate a descriptor. Returns the index, or -1 if none left.
    SLANG_FORCE_INLINE int allocate();
        /// Allocate a number of descriptors. Returns the start index (or -1 if not possible)
    SLANG_FORCE_INLINE int allocate(int numDescriptors);

        ///
    SLANG_FORCE_INLINE int placeAt(int index);

        /// Deallocates all allocations, and starts allocation from the start of the underlying heap again
    SLANG_FORCE_INLINE void deallocateAll() { m_currentIndex = 0; }

        /// Get the size of each
    SLANG_FORCE_INLINE int getDescriptorSize() const { return m_descriptorSize; }

        /// Get the GPU heap start
    SLANG_FORCE_INLINE D3D12_GPU_DESCRIPTOR_HANDLE getGpuStart() const { return m_heap->GetGPUDescriptorHandleForHeapStart(); }
        /// Get the CPU heap start
    SLANG_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE getCpuStart() const { return m_heap->GetCPUDescriptorHandleForHeapStart(); }

        /// Get the GPU handle at the specified index
    SLANG_FORCE_INLINE D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle(int index) const;
        /// Get the CPU handle at the specified index
    SLANG_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(int index) const;

        /// Get the underlying heap
    SLANG_FORCE_INLINE ID3D12DescriptorHeap* getHeap() const { return m_heap; }

        /// Ctor
    D3D12DescriptorHeap();

protected:
    Slang::ComPtr<ID3D12DescriptorHeap> m_heap;    ///< The underlying heap being allocated from
    int m_totalSize;                                ///< Total amount of allocations available on the heap
    int m_currentIndex;                        ///< The current descriptor
    int m_descriptorSize;                    ///< The size of each descriptor
};

/// A host-visible descriptor, used as "backing storage" for a view.
///
/// This type is intended to be used to represent descriptors that
/// are allocated and freed through a `HostVisibleDescriptorAllocator`.
struct D3D12HostVisibleDescriptor
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
};

/// An allocator for host-visible descriptors.
///
/// Unlike the `D3D12DescriptorHeap` type, this class allows for both
/// allocation and freeing of descriptors, by maintaining a free list.
///
class D3D12HostVisibleDescriptorAllocator
{
    ID3D12Device*                           m_device;
    int                                     m_chunkSize;
    D3D12_DESCRIPTOR_HEAP_TYPE              m_type;

    D3D12DescriptorHeap                     m_heap;
    Slang::VirtualObjectPool m_allocator;

public:
    D3D12HostVisibleDescriptorAllocator()
    {}

    Slang::Result init(ID3D12Device* device, int chunkSize, D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        m_device = device;
        m_chunkSize = chunkSize;
        m_type = type;

        SLANG_RETURN_ON_FAIL(m_heap.init(m_device, m_chunkSize, m_type, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
        m_allocator.initPool(m_chunkSize);
        return SLANG_OK;
    }

    SLANG_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(int index) const
    {
        return m_heap.getCpuHandle(index);
    }

    int allocate(int count)
    {
        return m_allocator.alloc(count);
    }

    Slang::Result allocate(D3D12HostVisibleDescriptor* outDescriptor)
    {
        // TODO: this allocator would take some work to make thread-safe

        int index = m_allocator.alloc(1);
        if(index < 0)
        {
            assert(!"descriptor allocation failed");
            return SLANG_FAIL;
        }

        D3D12HostVisibleDescriptor descriptor;
        descriptor.cpuHandle = m_heap.getCpuHandle(index);

        *outDescriptor = descriptor;
        return SLANG_OK;
    }

    void free(int index, int count)
    {
        m_allocator.free(index, count);
    }

    void free(D3D12HostVisibleDescriptor descriptor)
    {
        auto index =
            (int)(descriptor.cpuHandle.ptr - m_heap.getCpuStart().ptr) / m_heap.getDescriptorSize();
        free(index, 1);
    }
};

// ---------------------------------------------------------------------------
int D3D12DescriptorHeap::allocate()
{
    assert(m_currentIndex < m_totalSize);
    if (m_currentIndex < m_totalSize)
    {
        return m_currentIndex++;
    }
    return -1;
}
// ---------------------------------------------------------------------------
int D3D12DescriptorHeap::allocate(int numDescriptors)
{
    assert(m_currentIndex + numDescriptors <= m_totalSize);
    if (m_currentIndex + numDescriptors <= m_totalSize)
    {
        const int index = m_currentIndex;
        m_currentIndex += numDescriptors;
        return index;
    }
    return -1;
}
// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE int D3D12DescriptorHeap::placeAt(int index)
{
    assert(index >= 0 && index < m_totalSize);
    m_currentIndex = index + 1;
    return index;
}

// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::getCpuHandle(int index) const
{
    assert(index >= 0 && index < m_totalSize);
    D3D12_CPU_DESCRIPTOR_HANDLE start = m_heap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE dst;
    dst.ptr = start.ptr + m_descriptorSize * index;
    return dst;
}
// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::getGpuHandle(int index) const
{
    assert(index >= 0 && index < m_totalSize);
    D3D12_GPU_DESCRIPTOR_HANDLE start = m_heap->GetGPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE dst;
    dst.ptr = start.ptr + m_descriptorSize * index;
    return dst;
}

} // namespace gfx


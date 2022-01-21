#include "renderer-shared.h"
#include "source/core/slang-basic.h"

namespace gfx
{
template <typename TDevice, typename TBufferResource>
class TransientResourceHeapBaseImpl : public TransientResourceHeapBase
{
public:
    void breakStrongReferenceToDevice() { m_device.breakStrongReference(); }

public:
    BreakableReference<TDevice> m_device;
    Slang::List<Slang::RefPtr<TBufferResource>> m_constantBuffers;
    Slang::List<Slang::RefPtr<TBufferResource>> m_stagingBuffers;

    Slang::Index m_constantBufferAllocCounter = 0;
    size_t m_constantBufferOffsetAllocCounter = 0;
    uint32_t m_alignment = 256;

    Result init(const ITransientResourceHeap::Desc& desc, uint32_t alignment, TDevice* device)
    {
        m_device = device;

        if (desc.constantBufferSize)
        {
            Slang::ComPtr<IBufferResource> bufferPtr;
            IBufferResource::Desc bufferDesc;
            bufferDesc.type = IResource::Type::Buffer;
            bufferDesc.defaultState = ResourceState::ConstantBuffer;
            bufferDesc.allowedStates =
                ResourceStateSet(ResourceState::ConstantBuffer, ResourceState::CopyDestination);
            bufferDesc.sizeInBytes = desc.constantBufferSize;
            bufferDesc.memoryType = MemoryType::Upload;
            SLANG_RETURN_ON_FAIL(
                m_device->createBufferResource(bufferDesc, nullptr, bufferPtr.writeRef()));
            m_constantBuffers.add(static_cast<TBufferResource*>(bufferPtr.get()));
        }

        m_version = getVersionCounter();
        getVersionCounter()++;
        return SLANG_OK;
    }

    static size_t alignUp(size_t value, uint32_t alignment)
    {
        return (value + alignment - 1) / alignment * alignment;
    }

    Result allocateStagingBuffer(size_t size, IBufferResource*& outBufferWeakPtr, ResourceState state)
    {
        Slang::ComPtr<IBufferResource> bufferPtr;
        IBufferResource::Desc bufferDesc;
        bufferDesc.type = IResource::Type::Buffer;
        bufferDesc.defaultState = state;
        bufferDesc.allowedStates =
            ResourceStateSet(ResourceState::CopyDestination, ResourceState::CopySource);
        if (state == ResourceState::General)
            bufferDesc.memoryType = MemoryType::Upload;
        else
            bufferDesc.memoryType = MemoryType::ReadBack;
        bufferDesc.sizeInBytes = size;
        SLANG_RETURN_ON_FAIL(
            m_device->createBufferResource(bufferDesc, nullptr, bufferPtr.writeRef()));
        m_stagingBuffers.add(static_cast<TBufferResource*>(bufferPtr.get()));
        outBufferWeakPtr = bufferPtr.get();
        return SLANG_OK;
    }

    Result allocateConstantBuffer(
        size_t size,
        IBufferResource*& outBufferWeakPtr,
        size_t& outOffset)
    {
        size_t bufferAllocOffset = alignUp(m_constantBufferOffsetAllocCounter, m_alignment);
        Slang::Index bufferId = -1;
        // Find first constant buffer from `m_constantBufferAllocCounter` that has enough space
        // for this allocation.
        for (Slang::Index i = m_constantBufferAllocCounter; i < m_constantBuffers.getCount(); i++)
        {
            auto cb = m_constantBuffers[i].Ptr();
            if (bufferAllocOffset + size <= cb->getDesc()->sizeInBytes)
            {
                bufferId = i;
                break;
            }
            bufferAllocOffset = 0;
        }
        // If we cannot find an existing constant buffer with sufficient free space,
        // create a new constant buffer.
        if (bufferId == -1)
        {
            Slang::ComPtr<IBufferResource> bufferPtr;
            IBufferResource::Desc bufferDesc;
            bufferDesc.type = IResource::Type::Buffer;
            bufferDesc.defaultState = ResourceState::ConstantBuffer;
            bufferDesc.allowedStates =
                ResourceStateSet(ResourceState::ConstantBuffer, ResourceState::CopyDestination);
            bufferDesc.memoryType = MemoryType::Upload;
            size_t lastConstantBufferSize = 0;
            if (m_constantBuffers.getCount())
            {
                lastConstantBufferSize = m_constantBuffers.getLast()->getDesc()->sizeInBytes;
            }
            bufferDesc.sizeInBytes = Slang::Math::Max(
                lastConstantBufferSize * 2, Slang::Math::Max(size, size_t(4 << 20)));
            SLANG_RETURN_ON_FAIL(
                m_device->createBufferResource(bufferDesc, nullptr, bufferPtr.writeRef()));
            bufferId = m_constantBuffers.getCount();
            bufferAllocOffset = 0;
            m_constantBuffers.add(static_cast<TBufferResource*>(bufferPtr.get()));
        }
        // Sub allocate from current constant buffer.
        outBufferWeakPtr = m_constantBuffers[bufferId].Ptr();
        outOffset = bufferAllocOffset;
        m_constantBufferAllocCounter = bufferId;
        m_constantBufferOffsetAllocCounter = bufferAllocOffset + size;
        return SLANG_OK;
    }

    void reset()
    {
        m_constantBufferAllocCounter = 0;
        m_constantBufferOffsetAllocCounter = 0;
        for (auto& stagingBuffer : m_stagingBuffers)
            stagingBuffer = nullptr;
        m_stagingBuffers.clear();
        m_version = getVersionCounter();
        getVersionCounter()++;
    }
};

} // namespace gfx

#include "renderer-shared.h"
#include "source/core/slang-basic.h"

namespace gfx
{
template <typename TDevice, typename TBufferResource>
class TransientResourceHeapBase
    : public ITransientResourceHeap
    , public Slang::ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    ITransientResourceHeap* getInterface(const Slang::Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ITransientResourceHeap)
            return static_cast<ITransientResourceHeap*>(this);
        return nullptr;
    }
    void breakStrongReferenceToDevice() { m_device.breakStrongReference(); }

public:
    BreakableReference<TDevice> m_device;
    Slang::List<Slang::RefPtr<TBufferResource>> m_constantBuffers;
    Slang::Index m_constantBufferAllocCounter = 0;
    size_t m_constantBufferOffsetAllocCounter = 0;
    uint32_t m_alignment = 256;
    uint64_t m_version;
    uint64_t getVersion() { return m_version; }
    uint64_t& getVersionCounter()
    {
        static uint64_t version = 1;
        return version;
    }

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
            bufferDesc.cpuAccessFlags = AccessFlag::Write;
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
            bufferDesc.cpuAccessFlags |= AccessFlag::Write;
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
        m_version = getVersionCounter();
        getVersionCounter()++;
    }
};

} // namespace gfx

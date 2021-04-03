#include "slang-gfx.h"
#include "source/core/slang-basic.h"

namespace gfx
{
template <typename TDevice, typename TBufferResource>
class TransientResourceHeapBase
    : public ITransientResourceHeap
    , public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ITransientResourceHeap* getInterface(const Slang::Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ITransientResourceHeap)
            return static_cast<ITransientResourceHeap*>(this);
        return nullptr;
    }

public:
    TDevice* m_device;
    Slang::List<Slang::RefPtr<TBufferResource>> m_constantBuffers;
    Slang::Index m_constantBufferAllocCounter = 0;
    size_t m_constantBufferOffsetAllocCounter = 0;
    uint64_t m_version;
    uint64_t getVersion() { return m_version; }
    uint64_t& getVersionCounter()
    {
        static uint64_t version = 1;
        return version;
    }

    Result init(const ITransientResourceHeap::Desc& desc, TDevice* device)
    {
        m_device = device;

        if (desc.constantBufferSize)
        {
            Slang::ComPtr<IBufferResource> bufferPtr;
            IBufferResource::Desc bufferDesc;
            bufferDesc.type = IResource::Type::Buffer;
            bufferDesc.setDefaults(IResource::Usage::ConstantBuffer);
            bufferDesc.init(desc.constantBufferSize);
            bufferDesc.cpuAccessFlags = IResource::AccessFlag::Write;
            SLANG_RETURN_ON_FAIL(m_device->createBufferResource(
                IResource::Usage::ConstantBuffer, bufferDesc, nullptr, bufferPtr.writeRef()));
            m_constantBuffers.add(static_cast<TBufferResource*>(bufferPtr.get()));
        }

        m_version = getVersionCounter();
        getVersionCounter()++;
        return SLANG_OK;
    }

    Result allocateConstantBuffer(
        size_t size,
        IBufferResource*& outBufferWeakPtr,
        size_t& outOffset)
    {
        size_t bufferAllocOffset = m_constantBufferOffsetAllocCounter;
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
            bufferDesc.setDefaults(IResource::Usage::ConstantBuffer);
            bufferDesc.cpuAccessFlags |= IResource::AccessFlag::Write;
            size_t lastConstantBufferSize = 0;
            if (m_constantBuffers.getCount())
            {
                lastConstantBufferSize = m_constantBuffers.getLast()->getDesc()->sizeInBytes;
            }
            bufferDesc.init(Slang::Math::Max(
                lastConstantBufferSize * 2, Slang::Math::Max(size, size_t(4 << 20))));
            SLANG_RETURN_ON_FAIL(m_device->createBufferResource(
                IResource::Usage::ConstantBuffer, bufferDesc, nullptr, bufferPtr.writeRef()));
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

// simple-render-pass-layout.h
#pragma once

// Provide a simple no-op implementation for `ITransientResourceHeap` for targets that
// already support version management.

#include "slang-gfx.h"

namespace gfx
{
template<typename TDevice, typename TCommandBuffer>
class SimpleTransientResourceHeap
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
    ComPtr<IBufferResource> m_constantBuffer;

public:
    Result init(TDevice* device, const ITransientResourceHeap::Desc& desc)
    {
        m_device = device;
        IBufferResource::Desc bufferDesc = {};
        bufferDesc.setDefaults(IResource::Usage::ConstantBuffer);
        bufferDesc.sizeInBytes = desc.constantBufferSize;
        bufferDesc.cpuAccessFlags = IResource::AccessFlag::Write;
        SLANG_RETURN_ON_FAIL(device->createBufferResource(
            IResource::Usage::ConstantBuffer, bufferDesc, nullptr, m_constantBuffer.writeRef()));
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandBuffer(ICommandBuffer** outCommandBuffer) override
    {
        Slang::RefPtr<TCommandBuffer> newCmdBuffer = new TCommandBuffer();
        newCmdBuffer->init(m_device);
        *outCommandBuffer = newCmdBuffer.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() override { return SLANG_OK; }
};
}

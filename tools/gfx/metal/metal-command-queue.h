// metal-command-queue.h
#pragma once

#include "metal-base.h"
#include "metal-device.h"

namespace gfx
{

using namespace Slang;

namespace  metal
{

class CommandQueueImpl
    : public ICommandQueue
    , public ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    ICommandQueue* getInterface(const Guid& guid);

public:
    Desc m_desc;
    RefPtr<DeviceImpl> m_renderer;
    MTL::CommandQueue* m_commandQueue = nullptr;
    ~CommandQueueImpl();

    void init(DeviceImpl* renderer);

    virtual SLANG_NO_THROW void SLANG_MCALL waitOnHost() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFenceValuesOnDevice(
        GfxCount fenceCount, IFence** fences, uint64_t* waitValues) override;

    void queueSubmitImpl(
        uint32_t count,
        ICommandBuffer* const* commandBuffers,
        IFence* fence,
        uint64_t valueToSignal);

    virtual SLANG_NO_THROW void SLANG_MCALL executeCommandBuffers(
        GfxCount count,
        ICommandBuffer* const* commandBuffers,
        IFence* fence,
        uint64_t valueToSignal) override;
};

} // namespace metal
} // namespace gfx

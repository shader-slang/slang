// metal-pipeline-state.h
#pragma once

#include "metal-base.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

class PipelineStateImpl : public PipelineStateBase
{
public:
    PipelineStateImpl(DeviceImpl* device);
    ~PipelineStateImpl();

    // Turns `m_device` into a strong reference.
    // This method should be called before returning the pipeline state object to
    // external users (i.e. via an `IPipelineState` pointer).
    void establishStrongDeviceReference();

    virtual void comFree() override;

    void init(const GraphicsPipelineStateDesc& inDesc);
    void init(const ComputePipelineStateDesc& inDesc);
    void init(const RayTracingPipelineStateDesc& inDesc);

    Result createMetalComputePipelineState();
    Result createMetalRenderPipelineState();

    virtual Result ensureAPIPipelineStateCreated() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;

    BreakableReference<DeviceImpl> m_device;

    MTL::RenderPipelineState* m_renderState = nullptr;
    MTL::ComputePipelineState* m_computeState = nullptr;
};

class RayTracingPipelineStateImpl : public PipelineStateImpl
{
public:
    Dictionary<String, Index> shaderGroupNameToIndex;
    Int shaderGroupCount;

    RayTracingPipelineStateImpl(DeviceImpl* device);

    virtual Result ensureAPIPipelineStateCreated() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

} // namespace metal
} // namespace gfx

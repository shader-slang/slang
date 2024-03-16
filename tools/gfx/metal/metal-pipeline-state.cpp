// metal-pipeline-state.cpp
#include "metal-pipeline-state.h"

#include "metal-device.h"
#include "metal-shader-program.h"
#include "metal-shader-object-layout.h"
#include "metal-vertex-layout.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

PipelineStateImpl::PipelineStateImpl(DeviceImpl* device)
{
    // Only weakly reference `device` at start.
    // We make it a strong reference only when the pipeline state is exposed to the user.
    // Note that `PipelineState`s may also be created via implicit specialization that
    // happens behind the scenes, and the user will not have access to those specialized
    // pipeline states. Only those pipeline states that are returned to the user needs to
    // hold a strong reference to `device`.
    m_device.setWeakReference(device);
}

PipelineStateImpl::~PipelineStateImpl()
{
}

void PipelineStateImpl::establishStrongDeviceReference() { m_device.establishStrongReference(); }

void PipelineStateImpl::comFree() { m_device.breakStrongReference(); }

void PipelineStateImpl::init(const GraphicsPipelineStateDesc& inDesc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::Graphics;
    pipelineDesc.graphics = inDesc;
    initializeBase(pipelineDesc);
}

void PipelineStateImpl::init(const ComputePipelineStateDesc& inDesc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::Compute;
    pipelineDesc.compute = inDesc;
    initializeBase(pipelineDesc);
}

void PipelineStateImpl::init(const RayTracingPipelineStateDesc& inDesc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::RayTracing;
    pipelineDesc.rayTracing.set(inDesc);
    initializeBase(pipelineDesc);
}

Result PipelineStateImpl::createMetalRenderPipelineState()
{
    MTL::RenderPipelineDescriptor* pd = MTL::RenderPipelineDescriptor::alloc()->init();
    auto programImpl = static_cast<ShaderProgramImpl*>(m_program.Ptr());
    if (programImpl)
    {
        SLANG_RETURN_ON_FAIL(programImpl->compileShaders(m_device));
    }

    const auto& programReflection = m_program->linkedProgram->getLayout();
    const auto& composedProgram = m_program->linkedProgram;
    for (SlangUInt i = 0; i < programReflection->getEntryPointCount(); ++i)
    {
        SlangStage stage = programReflection->getEntryPointByIndex(i)->getStage();
        if (stage == SLANG_STAGE_VERTEX)
        {
            ComPtr<slang::IBlob> metalCode;
            {
                ComPtr<slang::IBlob> diagnosticsBlob;
                SlangResult result = composedProgram->getEntryPointCode(i, 0, metalCode.writeRef(), diagnosticsBlob.writeRef());
                if (diagnosticsBlob)
                {
                    std::cout << diagnosticsBlob->getBufferPointer() << std::endl;
                }
                //MTL::Function* f = ...
                //RETURN_ON_FAIL(result);
                //pd->setVertexFunction();
            }
        }
        //pd->setFragmentFunction();
    }
    // pd->colorAttachments()->object(0)->setPixelFormat(...);
    // pd->setDepthAttachmentPixelFormat(...);
    // Set deftault viewport and scissor
    // Set default rasterization state
    // Set default framebuffer layout
    NS::Error* error;
    m_renderState = m_device->m_device->newRenderPipelineState(pd, &error);
    if (m_renderState == nullptr)
    {
        std::cout << error->localizedDescription()->utf8String() << std::endl;
        return SLANG_E_INVALID_ARG;
    }
    return SLANG_OK;
}

Result PipelineStateImpl::createMetalComputePipelineState()
{
    return SLANG_OK;
}

Result PipelineStateImpl::ensureAPIPipelineStateCreated()
{
    if (m_renderState)
        return SLANG_OK;

    switch (desc.type)
    {
    case PipelineType::Compute:
        return createMetalComputePipelineState();
    case PipelineType::Graphics:
        return createMetalRenderPipelineState();
    default:
        SLANG_UNREACHABLE("Unknown pipeline type.");
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL PipelineStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_OK;
}

RayTracingPipelineStateImpl::RayTracingPipelineStateImpl(DeviceImpl* device)
    : PipelineStateImpl(device)
{}

Result RayTracingPipelineStateImpl::ensureAPIPipelineStateCreated()
{
    return SLANG_OK;
}

Result RayTracingPipelineStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_OK;
}



} // namespace metal
} // namespace gfx

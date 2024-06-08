// metal-pipeline-state.cpp
#include "metal-pipeline-state.h"

#include "metal-device.h"
#include "metal-shader-program.h"
#include "metal-shader-object-layout.h"
#include "metal-vertex-layout.h"
#include "metal-util.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

PipelineStateImpl::PipelineStateImpl(DeviceImpl* device)
    : m_device(device)
{
}

PipelineStateImpl::~PipelineStateImpl()
{
}

void PipelineStateImpl::init(const GraphicsPipelineStateDesc& desc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::Graphics;
    pipelineDesc.graphics = desc;
    initializeBase(pipelineDesc);
}

void PipelineStateImpl::init(const ComputePipelineStateDesc& desc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::Compute;
    pipelineDesc.compute = desc;
    initializeBase(pipelineDesc);
}

void PipelineStateImpl::init(const RayTracingPipelineStateDesc& desc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::RayTracing;
    pipelineDesc.rayTracing.set(desc);
    initializeBase(pipelineDesc);
}

Result PipelineStateImpl::createMetalRenderPipelineState()
{
    NS::SharedPtr<MTL::RenderPipelineDescriptor> pd = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
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
    m_renderPipelineState = NS::TransferPtr(m_device->m_device->newRenderPipelineState(pd.get(), &error));
    if (!m_renderPipelineState)
    {
        std::cout << error->localizedDescription()->utf8String() << std::endl;
        return SLANG_E_INVALID_ARG;
    }
    return SLANG_OK;
}

Result PipelineStateImpl::createMetalComputePipelineState()
{
    auto programImpl = static_cast<ShaderProgramImpl*>(m_program.Ptr());
    if (!programImpl)
        return SLANG_FAIL;

    NS::SharedPtr<MTL::ComputePipelineDescriptor> pd = NS::TransferPtr(MTL::ComputePipelineDescriptor::alloc()->init());

    auto functionName = MetalUtil::createString(programImpl->m_entryPointNames[0].getBuffer());
    NS::SharedPtr<MTL::Function> function = NS::TransferPtr(programImpl->m_modules[0]->newFunction(functionName.get()));
    if (!function)
        return SLANG_FAIL;

    NS::Error *error;
    m_computePipelineState = NS::TransferPtr(m_device->m_device->newComputePipelineState(function.get(), &error));

    // Query thread group size for use during dispatch.
    SlangUInt threadGroupSize[3];
    programImpl->linkedProgram->getLayout()->getEntryPointByIndex(0)->getComputeThreadGroupSize(3, threadGroupSize);
    m_threadGroupSize = MTL::Size(threadGroupSize[0], threadGroupSize[1], threadGroupSize[2]);

    return m_computePipelineState ? SLANG_OK : SLANG_FAIL;
}

Result PipelineStateImpl::ensureAPIPipelineStateCreated()
{
    switch (desc.type)
    {
    case PipelineType::Compute:
        return m_computePipelineState ? SLANG_OK : createMetalComputePipelineState();
    case PipelineType::Graphics:
        return m_renderPipelineState ? SLANG_OK : createMetalRenderPipelineState();
    default:
        SLANG_UNREACHABLE("Unknown pipeline type.");
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL PipelineStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

RayTracingPipelineStateImpl::RayTracingPipelineStateImpl(DeviceImpl* device)
    : PipelineStateImpl(device)
{}

Result RayTracingPipelineStateImpl::ensureAPIPipelineStateCreated()
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result RayTracingPipelineStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    return SLANG_E_NOT_IMPLEMENTED;
}



} // namespace metal
} // namespace gfx

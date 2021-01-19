#include "renderer-shared.h"
#include "render-graphics-common.h"

using namespace Slang;

namespace gfx
{

IResource* BufferResource::getInterface(const Slang::Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResource ||
        guid == GfxGUID::IID_IBufferResource)
        return static_cast<IBufferResource*>(this);
    return nullptr;
}

SLANG_NO_THROW IResource::Type SLANG_MCALL BufferResource::getType() { return m_type; }
SLANG_NO_THROW IBufferResource::Desc* SLANG_MCALL BufferResource::getDesc() { return &m_desc; }


IResource* TextureResource::getInterface(const Slang::Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResource ||
        guid == GfxGUID::IID_ITextureResource)
        return static_cast<ITextureResource*>(this);
    return nullptr;
}

SLANG_NO_THROW IResource::Type SLANG_MCALL TextureResource::getType() { return m_type; }
SLANG_NO_THROW ITextureResource::Desc* SLANG_MCALL TextureResource::getDesc() { return &m_desc; }

gfx::StageType mapStage(SlangStage stage)
{
    switch( stage )
    {
    default:
        return gfx::StageType::Unknown;

    case SLANG_STAGE_AMPLIFICATION:     return gfx::StageType::Amplification;
    case SLANG_STAGE_ANY_HIT:           return gfx::StageType::AnyHit;
    case SLANG_STAGE_CALLABLE:          return gfx::StageType::Callable;
    case SLANG_STAGE_CLOSEST_HIT:       return gfx::StageType::ClosestHit;
    case SLANG_STAGE_COMPUTE:           return gfx::StageType::Compute;
    case SLANG_STAGE_DOMAIN:            return gfx::StageType::Domain;
    case SLANG_STAGE_FRAGMENT:          return gfx::StageType::Fragment;
    case SLANG_STAGE_GEOMETRY:          return gfx::StageType::Geometry;
    case SLANG_STAGE_HULL:              return gfx::StageType::Hull;
    case SLANG_STAGE_INTERSECTION:      return gfx::StageType::Intersection;
    case SLANG_STAGE_MESH:              return gfx::StageType::Mesh;
    case SLANG_STAGE_MISS:              return gfx::StageType::Miss;
    case SLANG_STAGE_RAY_GENERATION:    return gfx::StageType::RayGeneration;
    case SLANG_STAGE_VERTEX:            return gfx::StageType::Vertex;
    }
}

Result createProgramFromSlang(IRenderer* renderer, IShaderProgram::Desc const& originalDesc, IShaderProgram** outProgram)
{
    SlangInt targetIndex = 0;
    auto slangProgram = originalDesc.slangProgram;

    auto programLayout = slangProgram->getLayout(targetIndex);
    if(!programLayout)
        return SLANG_FAIL;

    Int entryPointCount = (Int) programLayout->getEntryPointCount();
    if(entryPointCount == 0)
        return SLANG_FAIL;

    List<IShaderProgram::KernelDesc> kernelDescs;
    List<ComPtr<slang::IBlob>> kernelBlobs;
    for( Int i = 0; i < entryPointCount; ++i )
    {
        ComPtr<slang::IBlob> entryPointCodeBlob;
        SLANG_RETURN_ON_FAIL(slangProgram->getEntryPointCode(i, targetIndex, entryPointCodeBlob.writeRef()));

        auto entryPointLayout = programLayout->getEntryPointByIndex(i);

        kernelBlobs.add(entryPointCodeBlob);

        IShaderProgram::KernelDesc kernelDesc;
        kernelDesc.codeBegin = entryPointCodeBlob->getBufferPointer();
        kernelDesc.codeEnd = (const char*) kernelDesc.codeBegin + entryPointCodeBlob->getBufferSize();
        kernelDesc.entryPointName = entryPointLayout->getName();
        kernelDesc.stage = mapStage(entryPointLayout->getStage());

        kernelDescs.add(kernelDesc);
    }
    SLANG_ASSERT(kernelDescs.getCount() == entryPointCount);

    IShaderProgram::Desc programDesc;
    programDesc.pipelineType = originalDesc.pipelineType;
    programDesc.slangProgram = slangProgram;
    programDesc.kernelCount = kernelDescs.getCount();
    programDesc.kernels = kernelDescs.getBuffer();

    return renderer->createProgram(programDesc, outProgram);
}

} // namespace gfx

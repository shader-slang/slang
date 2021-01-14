#pragma once

#include "tools/gfx/render.h"
#include "core/slang-basic.h"

namespace gfx
{

class GraphicsAPIRenderer : public IRenderer, public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    virtual SLANG_NO_THROW Result SLANG_MCALL getFeatures(
        const char** outFeatures, UInt bufferSize, UInt* outFeatureCount) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* featureName) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, IShaderObjectLayout** outLayout) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObjectLayout(
        slang::ProgramLayout* programLayout, IShaderObjectLayout** outLayout) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createShaderObject(IShaderObjectLayout* layout, IShaderObject** outObject) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObject(
        IShaderObjectLayout* rootLayout, IShaderObject** outObject) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindRootShaderObject(PipelineType pipelineType, IShaderObject* object) SLANG_OVERRIDE;
    void preparePipelineDesc(GraphicsPipelineStateDesc& desc);
    void preparePipelineDesc(ComputePipelineStateDesc& desc);
    IRenderer* getInterface(const Slang::Guid& guid);

protected:
    Slang::List<Slang::String> m_features;
};

struct GfxGUID
{
    static const Slang::Guid IID_ISlangUnknown;
    static const Slang::Guid IID_IDescriptorSetLayout;
    static const Slang::Guid IID_IDescriptorSet;
    static const Slang::Guid IID_IShaderProgram;
    static const Slang::Guid IID_IPipelineLayout;
    static const Slang::Guid IID_IPipelineState;
    static const Slang::Guid IID_IResourceView;
    static const Slang::Guid IID_ISamplerState;
    static const Slang::Guid IID_IResource;
    static const Slang::Guid IID_IBufferResource;
    static const Slang::Guid IID_ITextureResource;
    static const Slang::Guid IID_IInputLayout;
    static const Slang::Guid IID_IRenderer;
    static const Slang::Guid IID_IShaderObjectLayout;
    static const Slang::Guid IID_IShaderObject;
};

}

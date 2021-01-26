#pragma once

#include "tools/gfx/render.h"
#include "core/slang-basic.h"

namespace gfx
{
class GraphicsCommonProgramLayout;

class GraphicsCommonShaderProgram : public IShaderProgram, public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    IShaderProgram* getInterface(const Slang::Guid& guid);

    GraphicsCommonProgramLayout* getLayout() const { return m_layout; }

protected:
    ~GraphicsCommonShaderProgram();

private:
    friend class GraphicsAPIRenderer;
    ComPtr<slang::IComponentType>       m_slangProgram;
    Slang::RefPtr<GraphicsCommonProgramLayout> m_layout;
};

class GraphicsAPIRenderer : public IRenderer, public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    virtual SLANG_NO_THROW Result SLANG_MCALL getFeatures(
        const char** outFeatures, UInt bufferSize, UInt* outFeatureCount) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* featureName) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, IShaderObjectLayout** outLayout) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createShaderObject(IShaderObjectLayout* layout, IShaderObject** outObject) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObject(
        IShaderProgram* program,
        IShaderObject** outObject) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindRootShaderObject(PipelineType pipelineType, IShaderObject* object) SLANG_OVERRIDE;
    void preparePipelineDesc(GraphicsPipelineStateDesc& desc);
    void preparePipelineDesc(ComputePipelineStateDesc& desc);
    IRenderer* getInterface(const Slang::Guid& guid);

    Result initProgramCommon(
        GraphicsCommonShaderProgram*    program,
        IShaderProgram::Desc const&     desc);

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

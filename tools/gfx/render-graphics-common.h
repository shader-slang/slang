#pragma once

#include "tools/gfx/renderer-shared.h"
#include "core/slang-basic.h"
#include "tools/gfx/slang-context.h"

namespace gfx
{
class GraphicsCommonProgramLayout;

class GraphicsCommonShaderProgram : public ShaderProgramBase
{
public:
    GraphicsCommonProgramLayout* getLayout() const;
private:
    friend class GraphicsAPIRenderer;
    Slang::RefPtr<ShaderObjectLayoutBase> m_layout;
};

class GraphicsAPIRenderer : public RendererBase
{
public:
    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection*    typeLayout,
        ShaderObjectLayoutBase**        outLayout) SLANG_OVERRIDE;
    virtual Result createShaderObject(
        ShaderObjectLayoutBase* layout,
        IShaderObject**         outObject) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObject(
        IShaderProgram* program,
        IShaderObject** outObject) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindRootShaderObject(PipelineType pipelineType, IShaderObject* object) SLANG_OVERRIDE;
    void preparePipelineDesc(GraphicsPipelineStateDesc& desc);
    void preparePipelineDesc(ComputePipelineStateDesc& desc);

    Result initProgramCommon(
        GraphicsCommonShaderProgram*    program,
        IShaderProgram::Desc const&     desc);
};
}

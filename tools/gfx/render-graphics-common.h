#pragma once

#include "tools/gfx/render.h"

namespace gfx
{

class GraphicsAPIRenderer : public IRenderer, public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, ShaderObjectLayout** outLayout) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObjectLayout(
        slang::ProgramLayout* programLayout, ShaderObjectLayout** outLayout) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(ShaderObjectLayout* layout, ShaderObject** outObject) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObject(ShaderObjectLayout* rootLayout, ShaderObject** outObject) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Result SLANG_MCALL bindRootShaderObject(PipelineType pipelineType, ShaderObject* object) SLANG_OVERRIDE;
    void preparePipelineDesc(GraphicsPipelineStateDesc& desc);
    void preparePipelineDesc(ComputePipelineStateDesc& desc);
    IRenderer* getInterface(const Slang::Guid& guid);
};

}

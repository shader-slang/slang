#pragma once

#include "tools/gfx/render.h"

namespace gfx
{

class GraphicsAPIRenderer : public IRenderer, public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, ShaderObjectLayout** outLayout) SLANG_OVERRIDE;
    virtual Result createRootShaderObjectLayout(
        slang::ProgramLayout* programLayout, ShaderObjectLayout** outLayout) SLANG_OVERRIDE;
    virtual Result createShaderObject(ShaderObjectLayout* layout, ShaderObject** outObject) SLANG_OVERRIDE;
    virtual Result createRootShaderObject(ShaderObjectLayout* rootLayout, ShaderObject** outObject) SLANG_OVERRIDE;
    virtual Result bindRootShaderObject(PipelineType pipelineType,  ShaderObject* object) SLANG_OVERRIDE;
    void preparePipelineDesc(GraphicsPipelineStateDesc& desc);
    void preparePipelineDesc(ComputePipelineStateDesc& desc);
    IRenderer* getInterface(const Slang::Guid& guid);
};

}

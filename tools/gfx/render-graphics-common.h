#pragma once

#include "tools/gfx/render.h"

namespace gfx
{

class GraphicsAPIRenderer : public Renderer
{
public:
    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, ShaderObjectLayout** outLayout) SLANG_OVERRIDE;
    virtual Result createRootShaderObjectLayout(
        slang::ProgramLayout* programLayout, ShaderObjectLayout** outLayout) SLANG_OVERRIDE;
    virtual Result createShaderObject(ShaderObjectLayout* layout, ShaderObject** outObject) SLANG_OVERRIDE;
    virtual Result createRootShaderObject(ShaderObjectLayout* rootLayout, ShaderObject** outObject) SLANG_OVERRIDE;
    virtual Result bindRootShaderObject(PipelineType pipelineType,  ShaderObject* object) SLANG_OVERRIDE;
    void preparePipelineDesc(GraphicsPipelineStateDesc& desc);
    void preparePipelineDesc(ComputePipelineStateDesc& desc);
};

}

// slang-support.h
#pragma once

#include "render.h"

#include <slang.h>

#include "shader-input-layout.h"

namespace renderer_test {

ShaderCompiler* createSlangShaderCompiler(
    ShaderCompiler*     innerCompiler,
    SlangSourceLanguage sourceLanguage,
    SlangCompileTarget  target);

    /// Create the texture resource using the renderer
SlangResult generateTextureResource(const InputTextureDesc& inputDesc, int bindFlags, Renderer* renderer, Slang::RefPtr<TextureResource>& textureOut);

    /// Create the buffer resource using the renderer
SlangResult createInputBufferResource(const InputBufferDesc& inputDesc, bool isOutput, size_t bufferSize, const void* initData, Renderer* renderer, Slang::RefPtr<BufferResource>& bufferOut);

SlangResult createBindingSetDesc(ShaderInputLayoutEntry* srcEntries, int numEntries, Renderer* renderer, BindingState::Desc& descOut);

    /// Create binding set from the layout
SlangResult createBindingSetDesc(const ShaderInputLayout& layout, Renderer* renderer, BindingState::Desc& descOut);

    /// Write out the contents of buffers marked as output in layout to file with 'filename'
SlangResult serializeBindingOutput(const ShaderInputLayout& layout, BindingState* bindingState, Renderer* renderer, const char* fileName);

} // renderer_test

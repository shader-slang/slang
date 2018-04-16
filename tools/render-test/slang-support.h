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

} // renderer_test

// shader-renderer-util.h
#pragma once

#include "slang-gfx.h"
#include "shader-input-layout.h"

namespace renderer_test {

using namespace Slang;

ComPtr<ISamplerState> _createSamplerState(IDevice* device, const InputSamplerDesc& srcDesc);

/// Utility class containing functions that construct items on the renderer using the ShaderInputLayout representation
struct ShaderRendererUtil
{
        /// Generate a texture using the InputTextureDesc and construct a TextureResource using the Renderer with the contents
    static Slang::Result generateTextureResource(
        const InputTextureDesc& inputDesc,
        ResourceState defaultState,
        IDevice* device,
        ComPtr<ITextureResource>& textureOut);

        /// Create texture resource using inputDesc, and texData to describe format, and contents
    static Slang::Result createTextureResource(
        const InputTextureDesc& inputDesc,
        const TextureData& texData,
        ResourceState defaultState,
        IDevice* device,
        ComPtr<ITextureResource>& textureOut);

        /// Create the BufferResource using the renderer from the contents of inputDesc
    static Slang::Result createBufferResource(
        const InputBufferDesc& inputDesc,
        size_t bufferSize,
        const void* initData,
        IDevice* device,
        ComPtr<IBufferResource>& bufferOut);
};

} // renderer_test

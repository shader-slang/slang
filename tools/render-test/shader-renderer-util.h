// shader-renderer-util.h
#pragma once

#include "render.h"
#include "shader-input-layout.h"

namespace renderer_test {

/// Utility class containing functions that construct items on the renderer using the ShaderInputLayout representation 
struct ShaderRendererUtil 
{
        /// Generate a texture using the InputTextureDesc and construct a TextureResource using the Renderer with the contents
    static Slang::Result generateTextureResource(const InputTextureDesc& inputDesc, int bindFlags, Renderer* renderer, Slang::RefPtr<TextureResource>& textureOut);

        /// Create texture resource using inputDesc, and texData to describe format, and contents
    static Slang::Result createTextureResource(const InputTextureDesc& inputDesc, const TextureData& texData, int bindFlags, Renderer* renderer, Slang::RefPtr<TextureResource>& textureOut);
    
        /// Create the BufferResource using the renderer from the contents of inputDesc
    static Slang::Result createBufferResource(const InputBufferDesc& inputDesc, bool isOutput, size_t bufferSize, const void* initData, Renderer* renderer, Slang::RefPtr<BufferResource>& bufferOut);

        /// Create BindingState::Desc from the contents of layout
    static Slang::Result createBindingStateDesc(const ShaderInputLayout& layout, Renderer* renderer, BindingState::Desc& descOut);
        /// Create BindingState::Desc from a list of ShaderInputLayout entries
    static Slang::Result createBindingStateDesc(ShaderInputLayoutEntry* srcEntries, int numEntries, Renderer* renderer, BindingState::Desc& descOut);

        /// Get the binding register associated with this binding (or -1 if none defined)
    static BindingState::BindingRegister calcBindingRegister(Renderer* renderer, const ShaderInputLayoutEntry& entry);

};

} // renderer_test

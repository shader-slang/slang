// shader-renderer-util.h
#pragma once

#include "render.h"
#include "shader-input-layout.h"

namespace renderer_test {

using namespace Slang;

struct BindingStateImpl : public Slang::RefObject
{
        /// A register set consists of one or more contiguous indices.
        /// To be valid index >= 0 and size >= 1
    struct RegisterRange
    {
            /// True if contains valid contents
        bool isValid() const { return size > 0; }
            /// True if valid single value
        bool isSingle() const { return size == 1; }
            /// Get as a single index (must be at least one index)
        int getSingleIndex() const { return (size == 1) ? index : -1; }
            /// Return the first index
        int getFirstIndex() const { return (size > 0) ? index : -1; }
            /// True if contains register index
        bool hasRegister(int registerIndex) const { return registerIndex >= index && registerIndex < index + size; }

        static RegisterRange makeInvalid() { return RegisterRange{ -1, 0 }; }
        static RegisterRange makeSingle(int index) { return RegisterRange{ int16_t(index), 1 }; }
        static RegisterRange makeRange(int index, int size) { return RegisterRange{ int16_t(index), uint16_t(size) }; }

        int16_t index;              ///< The base index
        uint16_t size;              ///< The amount of register indices
    };

    void apply(Renderer* renderer, PipelineType pipelineType);

    struct OutputBinding
    {
        RefPtr<Resource>    resource;
        Slang::UInt         entryIndex;
    };
    List<OutputBinding> outputBindings;

    RefPtr<PipelineLayout>  pipelineLayout;
    RefPtr<DescriptorSet>   descriptorSet;
    int m_numRenderTargets = 1;
};

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
    static Slang::Result createBindingState(const ShaderInputLayout& layout, Renderer* renderer, BufferResource* addedConstantBuffer, BindingStateImpl** outBindingState);

        /// Get the binding register associated with this binding (or -1 if none defined)
    static BindingStateImpl::RegisterRange calcRegisterRange(Renderer* renderer, const ShaderInputLayoutEntry& entry);

private:
        /// Create BindingState::Desc from a list of ShaderInputLayout entries
    static Slang::Result _createBindingState(ShaderInputLayoutEntry* srcEntries, int numEntries, Renderer* renderer, BufferResource* addedConstantBuffer, BindingStateImpl** outBindingState);
};

} // renderer_test

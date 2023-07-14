// slang-hlsl-to-vulkan-layout-options.h
#ifndef SLANG_HLSL_TO_VULKAN_LAYOUT_OPTIONS_H
#define SLANG_HLSL_TO_VULKAN_LAYOUT_OPTIONS_H

#include "../core/slang-basic.h"
#include "../core/slang-name-value.h"

namespace Slang
{

/*
For support features similar to described here..

https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#descriptors

Options that allow for infering Vulkan bindings based on HLSL register bindings
  */
struct HLSLToVulkanLayoutOptions : public RefObject
{
public:

    static const Index kInvalidShift = Index(0x80000000);

        /// For holding combination of set and index for binding
    struct Binding
    {
        bool isSet() const { return set >= 0 && index >= 0;}
        void reset() { set = -1; index = -1; }
        bool isInvalid() const { return !isSet(); }

        Index set = -1;
        Index index = -1;
    };

    // https://github.com/KhronosGroup/glslang/wiki/HLSL-FAQ
    // {b|s|t|u} 
    enum class Kind
    {
        Invalid = -1,

            /// Unordered access view (u) 
            ///
            /// RWByteAddressBuffer/RWStructuredBuffer
            /// Append/ConsumeStructuredBuffer
            /// RWBuffer
            /// RWTextureXD/Array
        UnorderedAccess,    

            /// Sampler (s)
            ///
            /// SamplerXD
            /// SamplerState/SamplerComparisonState
        Sampler,

            /// Shader Resource (t)
            ///
            /// TextureXD/Array
            /// ByteAddressBuffer/StructuredBuffer/Buffer/TBuffer
        ShaderResource,            

            /// Constant buffer (b)
            /// 
            /// ConstantBufferViews, CBuffer
        ConstantBuffer,     

        CountOf,
    };

    struct Key
    {
        typedef Key ThisType;

        bool operator==(const ThisType& rhs) const { return kind == rhs.kind && set == rhs.set; }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        HashCode getHashCode() const { return combineHash(Slang::getHashCode(kind), Slang::getHashCode(set)); }

        Kind kind;          ///< The kind this entry is for
        Index set;          ///< The set this shift is associated with 
    };

        /// Set the the all option for the kind
    void setAllShift(Kind kind, Index shift);

        /// Set the shift for kind/set
    void setShift(Kind kind, Index set, Index shift);

        /// Get the shift. Returns kInvalidShift if no shift is found
    Index getShift(Kind kind, Index set) const;

        /// True as global binds set
    bool hasGlobalsBinding() const { return m_globalsBinding.isSet(); }

        /// True if holds state such that vulkan bindings can be inferred from HLSL bindings
    bool canInferBindings() const;

        /// Given an kind and a binding infer the vulkan binding.
        /// Will return an invalid binding if one is not found
    Binding inferBinding(Kind kind, const Binding& inBinding) const;

        /// Reset state such that all options are set to their default. The same state as when 
        /// originally constructed
    void reset();

        /// Returns true if any state is set
    bool hasState() const;

        /// Returns true if contains default reset state. If so it can in effect be ignored
    bool isReset() const { return !hasState(); }

        /// Set the global binding
    void setGlobalsBinding(Index set, Index bindingIndex) { setGlobalsBinding(Binding{set, bindingIndex}); }
        /// Set the global bindings
    void setGlobalsBinding(const Binding& binding);
        /// Get the globals binding
    const Binding& getGlobalsBinding() const { return m_globalsBinding; }

        /// Ctor
    HLSLToVulkanLayoutOptions();
    
        /// Get information about the different kinds
    static ConstArrayView<NamesDescriptionValue> getKindInfos();

        /// Given a paramCategory get the kind. Returns Kind::Invalid if not an applicable category
    static Kind getKind(slang::ParameterCategory param);

protected:
    Binding m_globalsBinding;

    Index m_allShifts[Count(Kind::CountOf)];

        /// Maps a key to the amount of shift
    Dictionary<Key, Index> m_shifts;
};

} // namespace Slang

#endif

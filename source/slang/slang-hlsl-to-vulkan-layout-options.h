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

    // {b|s|t|u} 
    enum class Kind
    {
        Invalid = -1,

        Buffer,             ///< Buffer 
        Sampler,            ///< Sampler
        Texture,            ///< Texture
        Uniform,            ///< Uniform

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

        /// Returns true if contains default information. If so it can in effect be ignored
    bool isDefault() const;

        /// True as global binds set
    bool hasGlobalsBinding() const { return m_globalsBinding >= 0 && m_globalsBindingSet >= 0; }

        /// Ctor
    HLSLToVulkanLayoutOptions();

        /// Get information about the different kinds
    static ConstArrayView<NamesDescriptionValue> getKindInfos();

        /// Given a paramCategory get the kind. Returns Kind::Invalid if not an applicable category
    static Kind getKind(slang::ParameterCategory param);

    Index m_globalsBinding = -1;
    Index m_globalsBindingSet = -1;

    Index m_allShifts[Count(Kind::CountOf)];

        /// Maps a key to the amount of shift
    Dictionary<Key, Index> m_shifts;
};

} // namespace Slang

#endif

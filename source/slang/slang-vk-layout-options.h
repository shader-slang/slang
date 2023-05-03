// slang-vk-layout-options.h
#ifndef SLANG_VK_LAYOUT_OPTIONS_H
#define SLANG_VK_LAYOUT_OPTIONS_H

#include "../core/slang-basic.h"

#include "../core/slang-name-value.h"

namespace Slang
{

/*
  https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#descriptors
*/
struct VulkanLayoutOptions : public RefObject
{
public:

    // {b|s|t|u} 
    enum class Kind
    {
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
        Index set;          ///< If -1 this is the shift for all of this kind
    };

        /// Set the the all option for the kind
    void setAllShift(Kind kind, Index shift);

        /// Set the shift for kind/set
    void setShift(Kind kind, Index set, Index shift);

        /// Get the shift. If not found returns 0 meaning don't shift
    Index getShift(Kind kind, Index set) const;

        /// Returns true if contains default information. If so it can in effect be ignored
    bool isDefault() const;

    static ConstArrayView<NamesDescriptionValue> getKindInfos();

    Index m_globalsBinding = -1;
    Index m_globalsBindingSet = -1;

    Index m_allShifts[Count(Kind::CountOf)] = { 0 };

        /// Maps a key to the amount of shift
    Dictionary<Key, Index> m_shifts;
};

} // namespace Slang

#endif

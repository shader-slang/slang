// slang-vk-layout-options.cpp

#include "slang-vk-layout-options.h"

namespace Slang {

namespace { // anonymous

typedef VulkanLayoutOptions::Kind ShiftKind;

/* {b|s|t|u} */

static NamesDescriptionValue s_vulkanShiftKinds[] =
{
    { ValueInt(ShiftKind::Buffer),  "b", "Vulkan Buffer resource" },
    { ValueInt(ShiftKind::Sampler), "s", "Vulkan Sampler resource" },
    { ValueInt(ShiftKind::Texture), "t", "Vulkan Sampler resource" },
    { ValueInt(ShiftKind::Uniform), "u", "Vulkan Uniform resource" },
};

} // anonymous

/* static */ConstArrayView<NamesDescriptionValue> VulkanLayoutOptions::getKindInfos()
{
    return makeConstArrayView(s_vulkanShiftKinds);
}

   /// Set the the all option for the kind
void VulkanLayoutOptions::setAllShift(Kind kind, Index shift)
{
    // We try to follow the convention, of the *last* entry set is the one used.
    // If there a "all" set, we remove everything for the kind.

    // Find all the entries for the kind
    List<Key> keys;
    for (auto& pair : m_shifts)
    {
        if (pair.key.kind == kind)
        {
            keys.add(pair.key);
        }
    }
    // Remove them all
    for (auto& key : keys)
    {
        m_shifts.remove(key);
    }

    m_allShifts[Index(kind)] = shift;
}

void VulkanLayoutOptions::setShift(Kind kind, Index set, Index shift)
{
    SLANG_ASSERT(shift != kInvalidShift);

    Key key{ kind, set };
    m_shifts.add(key, shift);
}

Index VulkanLayoutOptions::getShift(Kind kind, Index set) const
{
    if (auto ptr = m_shifts.tryGetValue(Key{ kind, set }))
    {
        return *ptr;
    }

    return m_allShifts[Index(kind)];
}

bool VulkanLayoutOptions::isDefault() const
{
    // If any all shift is set it's not default
    for (auto shift : m_allShifts)
    {
        if (shift != kInvalidShift)
        {
            return false;
        }
    }

    // If any has a non zero shift, it's not default
    for (auto& pair : m_shifts)
    {
        // We need a value that is non zero...
        if (pair.value)
        {
            return false;
        }
    }

    // If either has been set it's not default
    return m_globalsBinding >= 0 || m_globalsBindingSet >= 0;
}

/* static */VulkanLayoutOptions::Kind VulkanLayoutOptions::getKind(slang::ParameterCategory param)
{
    typedef slang::ParameterCategory ParameterCategory;

    switch (param)
    {
        case ParameterCategory::Mixed:
        {
            // TODO(JS):
            // Hmm, is this TextureSampler?
            return Kind::Invalid;
        }
        case ParameterCategory::Uniform:
        case ParameterCategory::ConstantBuffer: 
        {
            return Kind::Uniform;
        }
        case ParameterCategory::ShaderResource:     return Kind::Texture;
        case ParameterCategory::UnorderedAccess:    return Kind::Buffer;
        case ParameterCategory::SamplerState:       return Kind::Sampler;
        
        default:
        {
            return Kind::Invalid;
        }
    }
}

} // namespace Slang

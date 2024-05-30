// metal-shader-object-layout.h
#pragma once

#include "metal-base.h"
#include "metal-device.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

enum
{
    kMaxDescriptorSets = 32,
};

class ShaderObjectLayoutImpl : public ShaderObjectLayoutBase
{
public:
	struct BindingOffset
	{
	};
    struct BindingRangeInfo
    {
        slang::BindingType bindingType;
        Index count;
        Index baseIndex;
        Index subObjectIndex;
        uint32_t bindingOffset;
        uint32_t offset;
        bool isSpecializable = false;
    };

    /// Offset information for a sub-object range
    struct SubObjectRangeOffset : BindingOffset
    {
        SubObjectRangeOffset() {}

        SubObjectRangeOffset(slang::VariableLayoutReflection* varLayout)
    //        : BindingOffset(varLayout)
        { }
        /// The offset for "pending" ordinary data related to this range
        uint32_t pendingOrdinaryData = 0;
    };

    /// Stride information for a sub-object range
    struct SubObjectRangeStride : BindingOffset
    {
        SubObjectRangeStride() {}

        SubObjectRangeStride(slang::TypeLayoutReflection* typeLayout)
        {
            if (auto pendingLayout = typeLayout->getPendingDataTypeLayout())
            {
                pendingOrdinaryData = (uint32_t)pendingLayout->getStride();
            }
        }

        /// The stride for "pending" ordinary data related to this range
        uint32_t pendingOrdinaryData = 0;
    };

    /// Information about a logical binding range as reported by Slang reflection
    struct SubObjectRangeInfo
    {
        /// The index of the binding range that corresponds to this sub-object range
        Index bindingRangeIndex;

        /// The layout expected for objects bound to this range (if known)
        RefPtr<ShaderObjectLayoutImpl> layout;

        /// The offset to use when binding the first object in this range
        SubObjectRangeOffset offset;

        /// Stride between consecutive objects in this range
        SubObjectRangeStride stride;
    };


    Index getBindingRangeCount() { return 0; }
    BindingRangeInfo const& getBindingRange(Index index) { return m_bindingRangeInfo[index]; }
    Index getResourceViewCount() { return 0; }
    Index getSamplerCount() { return 0; }
    Index getCombinedTextureSamplerCount() { return 0; }
    Index getSubObjectCount() { return 0; }

    SubObjectRangeInfo const& getSubObjectRange(Index index) { return m_subObjectRanges[index]; }
    List<SubObjectRangeInfo> const& getSubObjectRanges() { return m_subObjectRanges; }
protected:
    List<BindingRangeInfo> m_bindingRangeInfo;
	List<SubObjectRangeInfo> m_subObjectRanges;
};

class EntryPointLayout : public ShaderObjectLayoutImpl
{
    typedef ShaderObjectLayoutImpl Super;

public:
};

class RootShaderObjectLayout : public ShaderObjectLayoutImpl
{
    typedef ShaderObjectLayoutImpl Super;

public:
    ~RootShaderObjectLayout();
    static Result create(
        DeviceImpl* renderer,
        slang::IComponentType* program,
        slang::ProgramLayout* programLayout,
        RootShaderObjectLayout** outLayout);
protected:
public:
};

} // namespace metal
} // namespace gfx

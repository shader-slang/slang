// metal-shader-object-layout.cpp
#include "metal-shader-object-layout.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

ShaderObjectLayoutImpl::SubObjectRangeOffset::SubObjectRangeOffset(
    slang::VariableLayoutReflection* varLayout)
    : BindingOffset(varLayout)
{
}

ShaderObjectLayoutImpl::SubObjectRangeStride::SubObjectRangeStride(
    slang::TypeLayoutReflection* typeLayout)
    : BindingOffset(typeLayout)
{
}

Result ShaderObjectLayoutImpl::Builder::setElementTypeLayout(
    slang::TypeLayoutReflection* typeLayout)
{
    typeLayout = _unwrapParameterGroups(typeLayout, m_containerType);

    m_elementTypeLayout = typeLayout;

    m_totalOrdinaryDataSize = (uint32_t)typeLayout->getSize();
    if (m_totalOrdinaryDataSize > 0)
    {
        m_bufferCount++;
    }

    // Compute the binding ranges that are used to store
    // the logical contents of the object in memory.

    SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
    for (SlangInt r = 0; r < bindingRangeCount; ++r)
    {
        slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
        SlangInt count = typeLayout->getBindingRangeBindingCount(r);
        slang::TypeLayoutReflection* slangLeafTypeLayout =
            typeLayout->getBindingRangeLeafTypeLayout(r);

        BindingRangeInfo bindingRangeInfo;
        bindingRangeInfo.bindingType = slangBindingType;
        bindingRangeInfo.count = count;
        switch (slangBindingType)
        {
        case slang::BindingType::ConstantBuffer:
        case slang::BindingType::ParameterBlock:
        case slang::BindingType::ExistentialValue:
            bindingRangeInfo.baseIndex = m_subObjectCount;
            bindingRangeInfo.subObjectIndex = m_subObjectCount;
            m_subObjectCount += count;
            break;
        case slang::BindingType::RawBuffer:
        case slang::BindingType::MutableRawBuffer:
            bindingRangeInfo.baseIndex = m_bufferCount;
            if (slangLeafTypeLayout->getType()->getElementType() != nullptr)
            {
                // A structured buffer occupies both a resource slot and
                // a sub-object slot.
                bindingRangeInfo.subObjectIndex = m_subObjectCount;
                m_subObjectCount += count;
            }
            m_bufferCount += count;
            m_bufferRanges.add(r);
            break;
        case slang::BindingType::Sampler:
            bindingRangeInfo.baseIndex = m_samplerCount;
            m_samplerCount += count;
            m_samplerRanges.add(r);
            break;
        case slang::BindingType::Texture:
        case slang::BindingType::MutableTexture:
            bindingRangeInfo.baseIndex = m_textureCount;
            m_textureCount += count;
            m_textureRanges.add(r);
            break;
        case slang::BindingType::TypedBuffer:
        case slang::BindingType::MutableTypedBuffer:
            bindingRangeInfo.baseIndex = m_textureCount;
            m_textureCount += count;
            m_textureRanges.add(r);
            break;
        default:
            break;
        }

        // We'd like to extract the information on the Metal resource
        // index that this range should bind into.
        //
        // A binding range represents a logical member of the shader
        // object type, and it may encompass zero or more *descriptor
        // ranges* that describe how it is physically bound to pipeline
        // state.
        //
        // If the current binding range is backed by at least one descriptor
        // range then we can query the register offset of that descriptor
        // range. We expect that in the common case there will be exactly
        // one descriptor range, and we can extract the information easily.
        //
        // TODO: we might eventually need to special-case our handling
        // of combined texture-sampler ranges since they will need to
        // store two different offsets.
        //
        if (typeLayout->getBindingRangeDescriptorRangeCount(r) != 0)
        {
            // The Slang reflection information organizes the descriptor ranges
            // into "descriptor sets" but Metal has no notion like that so we
            // expect all ranges belong to a single set.
            //
            SlangInt descriptorSetIndex = typeLayout->getBindingRangeDescriptorSetIndex(r);
            SLANG_ASSERT(descriptorSetIndex == 0);

            SlangInt descriptorRangeIndex = typeLayout->getBindingRangeFirstDescriptorRangeIndex(r);
            auto registerOffset = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(
                descriptorSetIndex,
                descriptorRangeIndex);

            bindingRangeInfo.registerOffset = (uint32_t)registerOffset;
        }

        m_bindingRanges.add(bindingRangeInfo);
    }

    SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
    for (SlangInt r = 0; r < subObjectRangeCount; ++r)
    {
        SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
        auto& bindingRange = m_bindingRanges[bindingRangeIndex];

        auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
        slang::TypeLayoutReflection* slangLeafTypeLayout =
            typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

        SubObjectRangeInfo subObjectRange;
        subObjectRange.bindingRangeIndex = bindingRangeIndex;

        // We will use Slang reflection information to extract the offset and stride
        // information for each sub-object range.
        //
        subObjectRange.offset = SubObjectRangeOffset(typeLayout->getSubObjectRangeOffset(r));
        subObjectRange.stride = SubObjectRangeStride(slangLeafTypeLayout);

        // A sub-object range can either represent a sub-object of a known
        // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
        // *or* it can represent a sub-object of some existential type (e.g., `IBar`).
        //
        RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
        switch (slangBindingType)
        {
        default:
            {
                // In the case of `ConstantBuffer<X>` or `ParameterBlock<X>`
                // we can construct a layout from the element type directly.
                //
                auto elementTypeLayout = slangLeafTypeLayout->getElementTypeLayout();
                createForElementType(
                    m_renderer,
                    m_session,
                    elementTypeLayout,
                    subObjectLayout.writeRef());
            }
            break;
        }
        subObjectRange.layout = subObjectLayout;

        m_subObjectRanges.add(subObjectRange);
    }
    return SLANG_OK;
}

SlangResult ShaderObjectLayoutImpl::Builder::build(ShaderObjectLayoutImpl** outLayout)
{
    auto layout = RefPtr<ShaderObjectLayoutImpl>(new ShaderObjectLayoutImpl());
    SLANG_RETURN_ON_FAIL(layout->_init(this));

    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

slang::TypeLayoutReflection* ShaderObjectLayoutImpl::getParameterBlockTypeLayout()
{
    if (!m_parameterBlockTypeLayout)
    {
        m_parameterBlockTypeLayout = m_slangSession->getTypeLayout(
            m_elementTypeLayout->getType(),
            0,
            slang::LayoutRules::MetalArgumentBufferTier2);
    }
    return m_parameterBlockTypeLayout;
}

Result ShaderObjectLayoutImpl::createForElementType(
    RendererBase* renderer,
    slang::ISession* session,
    slang::TypeLayoutReflection* elementType,
    ShaderObjectLayoutImpl** outLayout)
{
    Builder builder(renderer, session);
    builder.setElementTypeLayout(elementType);
    return builder.build(outLayout);
}

Result ShaderObjectLayoutImpl::_init(Builder const* builder)
{
    auto renderer = builder->m_renderer;

    initBase(renderer, builder->m_session, builder->m_elementTypeLayout);

    m_bindingRanges = builder->m_bindingRanges;
    m_bufferRanges = builder->m_bufferRanges;
    m_textureRanges = builder->m_textureRanges;
    m_samplerRanges = builder->m_samplerRanges;

    m_bufferCount = builder->m_bufferCount;
    m_textureCount = builder->m_textureCount;
    m_samplerCount = builder->m_samplerCount;
    m_subObjectCount = builder->m_subObjectCount;
    m_subObjectRanges = builder->m_subObjectRanges;

    m_totalOrdinaryDataSize = builder->m_totalOrdinaryDataSize;

    m_containerType = builder->m_containerType;
    return SLANG_OK;
}

Result RootShaderObjectLayoutImpl::Builder::build(RootShaderObjectLayoutImpl** outLayout)
{
    RefPtr<RootShaderObjectLayoutImpl> layout = new RootShaderObjectLayoutImpl();
    SLANG_RETURN_ON_FAIL(layout->_init(this));

    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

void RootShaderObjectLayoutImpl::Builder::addGlobalParams(
    slang::VariableLayoutReflection* globalsLayout)
{
    setElementTypeLayout(globalsLayout->getTypeLayout());
}

void RootShaderObjectLayoutImpl::Builder::addEntryPoint(
    SlangStage stage,
    ShaderObjectLayoutImpl* entryPointLayout,
    slang::EntryPointLayout* slangEntryPoint)
{
    EntryPointInfo info;
    info.layout = entryPointLayout;
    info.offset = BindingOffset(slangEntryPoint->getVarLayout());
    m_entryPoints.add(info);
}

Result RootShaderObjectLayoutImpl::create(
    RendererBase* renderer,
    slang::IComponentType* program,
    slang::ProgramLayout* programLayout,
    RootShaderObjectLayoutImpl** outLayout)
{
    RootShaderObjectLayoutImpl::Builder builder(renderer, program, programLayout);
    builder.addGlobalParams(programLayout->getGlobalParamsVarLayout());

    SlangInt entryPointCount = programLayout->getEntryPointCount();
    for (SlangInt e = 0; e < entryPointCount; ++e)
    {
        auto slangEntryPoint = programLayout->getEntryPointByIndex(e);
        RefPtr<ShaderObjectLayoutImpl> entryPointLayout;
        SLANG_RETURN_ON_FAIL(ShaderObjectLayoutImpl::createForElementType(
            renderer,
            program->getSession(),
            slangEntryPoint->getTypeLayout(),
            entryPointLayout.writeRef()));
        builder.addEntryPoint(slangEntryPoint->getStage(), entryPointLayout, slangEntryPoint);
    }

    SLANG_RETURN_ON_FAIL(builder.build(outLayout));

    return SLANG_OK;
}

Result RootShaderObjectLayoutImpl::_init(Builder const* builder)
{
    auto renderer = builder->m_renderer;

    SLANG_RETURN_ON_FAIL(Super::_init(builder));

    m_program = builder->m_program;
    m_programLayout = builder->m_programLayout;
    m_entryPoints = builder->m_entryPoints;
    m_slangSession = m_program->getSession();

    return SLANG_OK;
}

} // namespace metal
} // namespace gfx

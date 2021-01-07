#include "render-graphics-common.h"
using namespace Slang;

namespace gfx
{

static const Slang::Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Slang::Guid IID_IRenderer = SLANG_UUID_IRenderer;

gfx::StageType translateStage(SlangStage slangStage)
{
    switch (slangStage)
    {
    default:
        SLANG_ASSERT(!"unhandled case");
        return gfx::StageType::Unknown;

#define CASE(FROM, TO)       \
    case SLANG_STAGE_##FROM: \
        return gfx::StageType::TO

        CASE(VERTEX, Vertex);
        CASE(HULL, Hull);
        CASE(DOMAIN, Domain);
        CASE(GEOMETRY, Geometry);
        CASE(FRAGMENT, Fragment);

        CASE(COMPUTE, Compute);

        CASE(RAY_GENERATION, RayGeneration);
        CASE(INTERSECTION, Intersection);
        CASE(ANY_HIT, AnyHit);
        CASE(CLOSEST_HIT, ClosestHit);
        CASE(MISS, Miss);
        CASE(CALLABLE, Callable);

#undef CASE
    }
}

class GraphicsCommonShaderObjectLayout : public ShaderObjectLayout
{
public:
    struct BindingRangeInfo
    {
        slang::BindingType bindingType;
        Index count;
        Index baseIndex;
        Index descriptorSetIndex;
        Index rangeIndexInDescriptorSet;
        //        Index   subObjectRangeIndex = -1;
    };

    struct SubObjectRangeInfo
    {
        RefPtr<GraphicsCommonShaderObjectLayout> layout;
        //        Index                       baseIndex;
        //        Index                       count;
        Index bindingRangeIndex;
    };

    struct DescriptorSetInfo : public RefObject
    {
        RefPtr<DescriptorSetLayout> layout;
        Slang::Int space = -1;
    };

    struct Builder
    {
    public:
        Builder(IRenderer* renderer)
            : m_renderer(renderer)
        {}

        List<BindingRangeInfo> m_bindingRanges;
        List<SubObjectRangeInfo> m_subObjectRanges;

        Index m_resourceViewCount = 0;
        Index m_samplerCount = 0;
        Index m_combinedTextureSamplerCount = 0;
        Index m_subObjectCount = 0;
        Index m_varyingInputCount = 0;
        Index m_varyingOutputCount = 0;

        struct DescriptorSetBuildInfo : public RefObject
        {
            List<DescriptorSetLayout::SlotRangeDesc> slotRangeDescs;
            Index space;
        };
        List<RefPtr<DescriptorSetBuildInfo>> m_descriptorSetBuildInfos;
        Dictionary<Index, Index> m_mapSpaceToDescriptorSetIndex;

        Index findOrAddDescriptorSet(Index space)
        {
            Index index;
            if (m_mapSpaceToDescriptorSetIndex.TryGetValue(space, index))
                return index;

            RefPtr<DescriptorSetBuildInfo> info = new DescriptorSetBuildInfo();
            info->space = space;

            index = m_descriptorSetBuildInfos.getCount();
            m_descriptorSetBuildInfos.add(info);

            m_mapSpaceToDescriptorSetIndex.Add(space, index);
            return index;
        }

        static DescriptorSlotType _mapDescriptorType(slang::BindingType slangBindingType)
        {
            switch (slangBindingType)
            {
            default:
                return DescriptorSlotType::Unknown;

#define CASE(FROM, TO)             \
    case slang::BindingType::FROM: \
        return DescriptorSlotType::TO

                CASE(Sampler, Sampler);
                CASE(CombinedTextureSampler, CombinedImageSampler);
                CASE(Texture, SampledImage);
                CASE(MutableTexture, StorageImage);
                CASE(TypedBuffer, UniformTexelBuffer);
                CASE(MutableTypedBuffer, StorageTexelBuffer);
                CASE(RawBuffer, UniformBuffer);
                CASE(MutableRawBuffer, StorageBuffer);
                CASE(InputRenderTarget, InputAttachment);
                CASE(InlineUniformData, InlineUniformBlock);
                CASE(RayTracingAccelerationStructure, RayTracingAccelerationStructure);
                CASE(ConstantBuffer, UniformBuffer);
                CASE(PushConstant, RootConstant);

#undef CASE
            }
        }

        slang::TypeLayoutReflection* unwrapParameterGroups(slang::TypeLayoutReflection* typeLayout)
        {
            for (;;)
            {
                if (!typeLayout->getType())
                {
                    if (auto elementTypeLayout = typeLayout->getElementTypeLayout())
                        typeLayout = elementTypeLayout;
                }

                switch (typeLayout->getKind())
                {
                default:
                    return typeLayout;

                case slang::TypeReflection::Kind::ConstantBuffer:
                case slang::TypeReflection::Kind::ParameterBlock:
                    typeLayout = typeLayout->getElementTypeLayout();
                    continue;
                }
            }
        }

        void _addDescriptorSets(
            slang::TypeLayoutReflection* typeLayout,
            slang::VariableLayoutReflection* varLayout = nullptr)
        {
            SlangInt descriptorSetCount = typeLayout->getDescriptorSetCount();
            for (SlangInt s = 0; s < descriptorSetCount; ++s)
            {
                auto descriptorSetIndex =
                    findOrAddDescriptorSet(typeLayout->getDescriptorSetSpaceOffset(s));
                auto descriptorSetInfo = m_descriptorSetBuildInfos[descriptorSetIndex];

                SlangInt descriptorRangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(s);
                for (SlangInt r = 0; r < descriptorRangeCount; ++r)
                {
                    auto slangBindingType = typeLayout->getDescriptorSetDescriptorRangeType(s, r);
                    auto gfxDescriptorType = _mapDescriptorType(slangBindingType);

                    DescriptorSetLayout::SlotRangeDesc descriptorRangeDesc;
                    descriptorRangeDesc.binding =
                        typeLayout->getDescriptorSetDescriptorRangeIndexOffset(s, r);
                    descriptorRangeDesc.count =
                        typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(s, r);
                    descriptorRangeDesc.type = gfxDescriptorType;

                    if (varLayout)
                    {
                        auto category = typeLayout->getDescriptorSetDescriptorRangeCategory(s, r);
                        descriptorRangeDesc.binding += varLayout->getOffset(category);
                    }

                    descriptorSetInfo->slotRangeDescs.add(descriptorRangeDesc);
                }
            }
        }

        Result setElementTypeLayout(slang::TypeLayoutReflection* typeLayout)
        {
            typeLayout = unwrapParameterGroups(typeLayout);

            m_elementTypeLayout = typeLayout;

            // First we will use the Slang layout information to allocate
            // the descriptor set layout(s) required to store values
            // of the given type.
            //
            _addDescriptorSets(typeLayout);

            // Next we will compute the binding ranges that are used to store
            // the logical contents of the object in memory. These will relate
            // to the descriptor ranges in the various sets, but not always
            // in a one-to-one fashion.

            SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
            for (SlangInt r = 0; r < bindingRangeCount; ++r)
            {
                slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                SlangInt count = typeLayout->getBindingRangeBindingCount(r);
                slang::TypeLayoutReflection* slangLeafTypeLayout =
                    typeLayout->getBindingRangeLeafTypeLayout(r);

                SlangInt descriptorSetIndex = typeLayout->getBindingRangeDescriptorSetIndex(r);
                SlangInt rangeIndexInDescriptorSet =
                    typeLayout->getBindingRangeFirstDescriptorRangeIndex(r);

                Index baseIndex = 0;
                switch (slangBindingType)
                {
                case slang::BindingType::ConstantBuffer:
                case slang::BindingType::ParameterBlock:
                case slang::BindingType::ExistentialValue:
                    baseIndex = m_subObjectCount;
                    m_subObjectCount += count;
                    break;

                case slang::BindingType::Sampler:
                    baseIndex = m_samplerCount;
                    m_samplerCount += count;
                    break;

                case slang::BindingType::CombinedTextureSampler:
                    baseIndex = m_combinedTextureSamplerCount;
                    m_combinedTextureSamplerCount += count;
                    break;

                case slang::BindingType::VaryingInput:
                    baseIndex = m_varyingInputCount;
                    m_varyingInputCount += count;
                    break;

                case slang::BindingType::VaryingOutput:
                    baseIndex = m_varyingOutputCount;
                    m_varyingOutputCount += count;
                    break;

                default:
                    baseIndex = m_resourceViewCount;
                    m_resourceViewCount += count;
                    break;
                }

                BindingRangeInfo bindingRangeInfo;
                bindingRangeInfo.bindingType = slangBindingType;
                bindingRangeInfo.count = count;
                //                bindingRangeInfo.descriptorSetIndex = descriptorSetIndex;
                //                bindingRangeInfo.rangeIndexInDescriptorSet = slotRangeIndex;
                //                bindingRangeInfo.subObjectRangeIndex = subObjectRangeIndex;
                bindingRangeInfo.baseIndex = baseIndex;
                bindingRangeInfo.descriptorSetIndex = descriptorSetIndex;
                bindingRangeInfo.rangeIndexInDescriptorSet = rangeIndexInDescriptorSet;

                m_bindingRanges.add(bindingRangeInfo);

#if 0
                SlangInt binding = typeLayout->getBindingRangeIndexOffset(r);
                SlangInt space = typeLayout->getBindingRangeSpaceOffset(r);
                SlangInt subObjectRangeIndex = typeLayout->getBindingRangeSubObjectRangeIndex(r);

                DescriptorSetLayout::SlotRangeDesc slotRange;
                slotRange.type = _mapDescriptorType(slangBindingType);
                slotRange.count = count;
                slotRange.binding = binding;

                Index descriptorSetIndex = findOrAddDescriptorSet(space);
                RefPtr<DescriptorSetBuildInfo> descriptorSetInfo = m_descriptorSetInfos[descriptorSetIndex];

                Index slotRangeIndex = descriptorSetInfo->slotRanges.getCount();
                descriptorSetInfo->slotRanges.add(slotRange);
#endif
            }

            SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
            for (SlangInt r = 0; r < subObjectRangeCount; ++r)
            {
                SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
                auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
                slang::TypeLayoutReflection* slangLeafTypeLayout =
                    typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

                // A sub-object range can either represent a sub-object of a known
                // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
                // (in which case we can pre-compute a layout to use, based on
                // the type `Foo`) *or* it can represent a sub-object of some
                // existential type (e.g., `IBar`) in which case we cannot
                // know the appropraite type/layout of sub-object to allocate.
                //
                RefPtr<GraphicsCommonShaderObjectLayout> subObjectLayout;
                if (slangBindingType != slang::BindingType::ExistentialValue)
                {
                    GraphicsCommonShaderObjectLayout::createForElementType(
                        m_renderer,
                        slangLeafTypeLayout->getElementTypeLayout(),
                        subObjectLayout.writeRef());
                }

                SubObjectRangeInfo subObjectRange;
                subObjectRange.bindingRangeIndex = bindingRangeIndex;
                subObjectRange.layout = subObjectLayout;

                m_subObjectRanges.add(subObjectRange);
            }

#if 0
            SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
            for(SlangInt r = 0; r < subObjectRangeCount; ++r)
            {

                // TODO: Still need a way to map the binding ranges for
                // the sub-object over so that they can be used to
                // set/get the sub-object as needed.
            }
#endif
            return SLANG_OK;
        }

        SlangResult build(GraphicsCommonShaderObjectLayout** outLayout)
        {
            RefPtr<GraphicsCommonShaderObjectLayout> layout =
                new GraphicsCommonShaderObjectLayout();
            SLANG_RETURN_ON_FAIL(layout->_init(this));

            *outLayout = layout.detach();
            return SLANG_OK;
        }

        IRenderer* m_renderer = nullptr;
        slang::TypeLayoutReflection* m_elementTypeLayout = nullptr;
    };

    static Result createForElementType(
        IRenderer* renderer,
        slang::TypeLayoutReflection* elementType,
        GraphicsCommonShaderObjectLayout** outLayout)
    {
        Builder builder(renderer);
        builder.setElementTypeLayout(elementType);
        return builder.build(outLayout);
    }

    List<RefPtr<DescriptorSetInfo>> const& getDescriptorSets() { return m_descriptorSets; }

    List<BindingRangeInfo> const& getBindingRanges() { return m_bindingRanges; }

    Index getBindingRangeCount() { return m_bindingRanges.getCount(); }

    BindingRangeInfo const& getBindingRange(Index index) { return m_bindingRanges[index]; }

    slang::TypeLayoutReflection* getElementTypeLayout() { return m_elementTypeLayout; }

    Index getResourceViewCount() { return m_resourceViewCount; }
    Index getSamplerCount() { return m_samplerCount; }
    Index getCombinedTextureSamplerCount() { return m_combinedTextureSamplerCount; }
    Index getSubObjectCount() { return m_subObjectCount; }

    SubObjectRangeInfo const& getSubObjectRange(Index index) { return m_subObjectRanges[index]; }
    List<SubObjectRangeInfo> const& getSubObjectRanges() { return m_subObjectRanges; }

    IRenderer* getRenderer() { return m_renderer; }

protected:
    Result _init(Builder const* builder)
    {
        auto renderer = builder->m_renderer;
        m_renderer = renderer;

        m_elementTypeLayout = builder->m_elementTypeLayout;
        m_bindingRanges = builder->m_bindingRanges;

        for (auto descriptorSetBuildInfo : builder->m_descriptorSetBuildInfos)
        {
            auto& slotRangeDescs = descriptorSetBuildInfo->slotRangeDescs;
            DescriptorSetLayout::Desc desc;
            desc.slotRangeCount = slotRangeDescs.getCount();
            desc.slotRanges = slotRangeDescs.getBuffer();

            RefPtr<DescriptorSetLayout> descriptorSetLayout;
            SLANG_RETURN_ON_FAIL(
                m_renderer->createDescriptorSetLayout(desc, descriptorSetLayout.writeRef()));

            RefPtr<DescriptorSetInfo> descriptorSetInfo = new DescriptorSetInfo();
            descriptorSetInfo->layout = descriptorSetLayout;
            descriptorSetInfo->space = descriptorSetBuildInfo->space;

            m_descriptorSets.add(descriptorSetInfo);
        }

        m_resourceViewCount = builder->m_resourceViewCount;
        m_samplerCount = builder->m_samplerCount;
        m_combinedTextureSamplerCount = builder->m_combinedTextureSamplerCount;
        m_subObjectCount = builder->m_subObjectCount;

        m_subObjectRanges = builder->m_subObjectRanges;

        return SLANG_OK;
    }

    IRenderer* m_renderer;
    List<RefPtr<DescriptorSetInfo>> m_descriptorSets;
    List<BindingRangeInfo> m_bindingRanges;
    slang::TypeLayoutReflection* m_elementTypeLayout;
    Index m_resourceViewCount = 0;
    Index m_samplerCount = 0;
    Index m_combinedTextureSamplerCount = 0;
    Index m_subObjectCount = 0;
    List<SubObjectRangeInfo> m_subObjectRanges;
};

class EntryPointLayout : public GraphicsCommonShaderObjectLayout
{
    typedef GraphicsCommonShaderObjectLayout Super;

public:
    struct VaryingInputInfo
    {};

    struct VaryingOutputInfo
    {};

    struct Builder : Super::Builder
    {
        Builder(IRenderer* renderer)
            : Super::Builder(renderer)
        {}

        Result build(EntryPointLayout** outLayout)
        {
            RefPtr<EntryPointLayout> layout = new EntryPointLayout();
            SLANG_RETURN_ON_FAIL(layout->_init(this));

            *outLayout = layout.detach();
            return SLANG_OK;
        }

        void _addEntryPointParam(slang::VariableLayoutReflection* entryPointParam)
        {
            auto slangStage = entryPointParam->getStage();
            auto typeLayout = entryPointParam->getTypeLayout();

            SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
            for (SlangInt r = 0; r < bindingRangeCount; ++r)
            {
                slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                SlangInt count = typeLayout->getBindingRangeBindingCount(r);

                switch (slangBindingType)
                {
                default:
                    break;

                case slang::BindingType::VaryingInput:
                    {
                        VaryingInputInfo info;

                        m_varyingInputs.add(info);
                    }
                    break;

                case slang::BindingType::VaryingOutput:
                    {
                        VaryingOutputInfo info;
                        m_varyingOutputs.add(info);
                    }
                    break;
                }
            }
        }

        void addEntryPointParams(slang::EntryPointLayout* entryPointLayout)
        {
            m_slangEntryPointLayout = entryPointLayout;

            setElementTypeLayout(entryPointLayout->getTypeLayout());

            m_stage = translateStage(entryPointLayout->getStage());
            _addEntryPointParam(entryPointLayout->getVarLayout());
            _addEntryPointParam(entryPointLayout->getResultVarLayout());
        }

        slang::EntryPointLayout* m_slangEntryPointLayout = nullptr;

        gfx::StageType m_stage;
        List<VaryingInputInfo> m_varyingInputs;
        List<VaryingOutputInfo> m_varyingOutputs;
    };

    Result _init(Builder const* builder)
    {
        auto renderer = builder->m_renderer;

        SLANG_RETURN_ON_FAIL(Super::_init(builder));

        m_slangEntryPointLayout = builder->m_slangEntryPointLayout;
        m_stage = builder->m_stage;
        m_varyingInputs = builder->m_varyingInputs;
        m_varyingOutputs = builder->m_varyingOutputs;

        return SLANG_OK;
    }

    List<VaryingInputInfo> const& getVaryingInputs() { return m_varyingInputs; }
    List<VaryingOutputInfo> const& getVaryingOutputs() { return m_varyingOutputs; }

    gfx::StageType getStage() const { return m_stage; }

    slang::EntryPointLayout* getSlangLayout() const { return m_slangEntryPointLayout; };

    slang::EntryPointLayout* m_slangEntryPointLayout;
    gfx::StageType m_stage;
    List<VaryingInputInfo> m_varyingInputs;
    List<VaryingOutputInfo> m_varyingOutputs;
};

class GraphicsCommonProgramLayout : public GraphicsCommonShaderObjectLayout
{
    typedef GraphicsCommonShaderObjectLayout Super;

public:
    struct EntryPointInfo
    {
        RefPtr<EntryPointLayout> layout;
        Index rangeOffset;
    };

    struct Builder : Super::Builder
    {
        Builder(IRenderer* renderer)
            : Super::Builder(renderer)
        {}

        Result build(GraphicsCommonProgramLayout** outLayout)
        {
            RefPtr<GraphicsCommonProgramLayout> layout = new GraphicsCommonProgramLayout();
            SLANG_RETURN_ON_FAIL(layout->_init(this));

            *outLayout = layout.detach();
            return SLANG_OK;
        }

        void addGlobalParams(slang::VariableLayoutReflection* globalsLayout)
        {
            setElementTypeLayout(globalsLayout->getTypeLayout());
        }

        void addEntryPoint(EntryPointLayout* entryPointLayout)
        {
            EntryPointInfo info;
            info.layout = entryPointLayout;

            if (m_descriptorSetBuildInfos.getCount())
            {
                info.rangeOffset = m_descriptorSetBuildInfos[0]->slotRangeDescs.getCount();
            }

            auto slangEntryPointLayout = entryPointLayout->getSlangLayout();
            _addDescriptorSets(
                slangEntryPointLayout->getTypeLayout(), slangEntryPointLayout->getVarLayout());

            m_entryPoints.add(info);
        }

        List<EntryPointInfo> m_entryPoints;
    };

    Slang::Int getRenderTargetCount() { return m_renderTargetCount; }

    PipelineLayout* getPipelineLayout() { return m_pipelineLayout; }

    Index findEntryPointIndex(gfx::StageType stage)
    {
        auto entryPointCount = m_entryPoints.getCount();
        for (Index i = 0; i < entryPointCount; ++i)
        {
            auto entryPoint = m_entryPoints[i];
            if (entryPoint.layout->getStage() == stage)
                return i;
        }
        return -1;
    }

    EntryPointInfo const& getEntryPoint(Index index) { return m_entryPoints[index]; }

    List<EntryPointInfo> const& getEntryPoints() const { return m_entryPoints; }

protected:
    Result _init(Builder const* builder)
    {
        auto renderer = builder->m_renderer;

        SLANG_RETURN_ON_FAIL(Super::_init(builder));

        m_entryPoints = builder->m_entryPoints;

        List<PipelineLayout::DescriptorSetDesc> pipelineDescriptorSets;
        _addDescriptorSetsRec(this, pipelineDescriptorSets);

#if 0
        _createInputLayout(builder);
#endif

        auto fragmentEntryPointIndex = findEntryPointIndex(gfx::StageType::Fragment);
        if (fragmentEntryPointIndex != -1)
        {
            auto fragmentEntryPoint = getEntryPoint(fragmentEntryPointIndex);
            m_renderTargetCount = fragmentEntryPoint.layout->getVaryingOutputs().getCount();
        }

        PipelineLayout::Desc pipelineLayoutDesc;
        pipelineLayoutDesc.renderTargetCount = m_renderTargetCount;
        pipelineLayoutDesc.descriptorSetCount = pipelineDescriptorSets.getCount();
        pipelineLayoutDesc.descriptorSets = pipelineDescriptorSets.getBuffer();

        SLANG_RETURN_ON_FAIL(
            renderer->createPipelineLayout(pipelineLayoutDesc, m_pipelineLayout.writeRef()));

        return SLANG_OK;
    }

    static void _addDescriptorSetsRec(
        GraphicsCommonShaderObjectLayout* layout,
        List<PipelineLayout::DescriptorSetDesc>& ioPipelineDescriptorSets)
    {
        for (auto descriptorSetInfo : layout->getDescriptorSets())
        {
            PipelineLayout::DescriptorSetDesc pipelineDescriptorSet;
            pipelineDescriptorSet.layout = descriptorSetInfo->layout;
            pipelineDescriptorSet.space = descriptorSetInfo->space;

            ioPipelineDescriptorSets.add(pipelineDescriptorSet);
        }

        // TODO: next we need to recurse into the "sub-objects" of `layout` and
        // add their descriptor sets as well.
    }

#if 0
    Result _createInputLayout(Builder const* builder)
    {
        auto renderer = builder->m_renderer;

        List<InputElementDesc> const& inputElements = builder->getInputElements();
        SLANG_RETURN_ON_FAIL(renderer->createInputLayout(inputElements.getBuffer(), inputElements.getCount(), m_inputLayout.writeRef()));

        return SLANG_OK;
    }
#endif

    List<EntryPointInfo> m_entryPoints;
    gfx::UInt m_renderTargetCount = 0;

    RefPtr<PipelineLayout> m_pipelineLayout;
};

class GraphicsCommonShaderObject : public ShaderObject
{
public:
    static Result create(
        IRenderer* renderer,
        GraphicsCommonShaderObjectLayout* layout,
        GraphicsCommonShaderObject** outShaderObject)
    {
        RefPtr<GraphicsCommonShaderObject> object = new GraphicsCommonShaderObject();
        SLANG_RETURN_ON_FAIL(object->init(renderer, layout));

        *outShaderObject = object.detach();
        return SLANG_OK;
    }

    IRenderer* getRenderer() { return m_layout->getRenderer(); }

    Index getEntryPointCount() { return 0; }

    ShaderObject* getEntryPoint(Index index) { return nullptr; }

    GraphicsCommonShaderObjectLayout* getLayout() { return m_layout; }

    slang::TypeLayoutReflection* getElementTypeLayout() { return m_layout->getElementTypeLayout(); }

    SlangResult setData(ShaderOffset const& offset, void const* data, size_t size)
    {
        IRenderer* renderer = getRenderer();

        char* dest = (char*)renderer->map(m_buffer, MapFlavor::HostWrite);
        memcpy(dest + offset.uniformOffset, data, size);
        renderer->unmap(m_buffer);

        return SLANG_OK;
    }

    virtual SlangResult setObject(ShaderOffset const& offset, ShaderObject* object) SLANG_OVERRIDE
    {
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        if (offset.bindingRangeIndex >= m_layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        // TODO: Is this reasonable to store the base index directly in the binding range?
        m_objects[bindingRange.baseIndex + offset.bindingArrayIndex] =
            reinterpret_cast<GraphicsCommonShaderObject*>(object);

        //        auto& subObjectRange =
        //        m_layout->getSubObjectRange(bindingRange.subObjectRangeIndex);
        //        m_objects[subObjectRange.baseIndex + offset.bindingArrayIndex] = object;

#if 0

        SLANG_ASSERT(bindingRange.descriptorSetIndex >= 0);
        SLANG_ASSERT(bindingRange.descriptorSetIndex < m_descriptorSets.getCount());
        auto& descriptorSet = m_descriptorSets[bindingRange.descriptorSetIndex];

        descriptorSet->setConstantBuffer(bindingRange.rangeIndexInDescriptorSet, offset.bindingArrayIndex, buffer);
        return SLANG_OK;
#else
        return SLANG_E_NOT_IMPLEMENTED;
#endif
    }

    virtual SlangResult getObject(ShaderOffset const& offset, ShaderObject** outObject) SLANG_OVERRIDE
    {
        SLANG_ASSERT(outObject);
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        if (offset.bindingRangeIndex >= m_layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        *outObject = m_objects[bindingRange.baseIndex + offset.bindingArrayIndex];

        //        auto& subObjectRange =
        //        m_layout->getSubObjectRange(bindingRange.subObjectRangeIndex); *outObject =
        //        m_objects[subObjectRange.baseIndex + offset.bindingArrayIndex];

        return SLANG_OK;

#if 0
        SLANG_ASSERT(bindingRange.descriptorSetIndex >= 0);
        SLANG_ASSERT(bindingRange.descriptorSetIndex < m_descriptorSets.getCount());
        auto& descriptorSet = m_descriptorSets[bindingRange.descriptorSetIndex];

        descriptorSet->setConstantBuffer(bindingRange.rangeIndexInDescriptorSet, offset.bindingArrayIndex, buffer);
        return SLANG_OK;
#endif
    }

    ShaderObject* getObject(ShaderOffset const& offset)
    {
        ShaderObject* object = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(getObject(offset, &object));
        return object;
    }

    SlangResult setResource(ShaderOffset const& offset, ResourceView* resourceView)
    {
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        if (offset.bindingRangeIndex >= m_layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        m_resourceViews[bindingRange.baseIndex + offset.bindingArrayIndex] = resourceView;
        return SLANG_OK;
    }

    SlangResult setSampler(ShaderOffset const& offset, SamplerState* sampler)
    {
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        if (offset.bindingRangeIndex >= m_layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        m_samplers[bindingRange.baseIndex + offset.bindingArrayIndex] = sampler;
        return SLANG_OK;
    }

    SlangResult setCombinedTextureSampler(
        ShaderOffset const& offset, ResourceView* textureView, SamplerState* sampler)
    {
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        if (offset.bindingRangeIndex >= m_layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        auto& slot = m_combinedTextureSamplers[bindingRange.baseIndex + offset.bindingArrayIndex];
        slot.textureView = textureView;
        slot.sampler = sampler;
        return SLANG_OK;
    }

protected:
    friend class ProgramVars;

    Result init(IRenderer* renderer, GraphicsCommonShaderObjectLayout* layout)
    {
        m_layout = layout;

        // If the layout tells us that there is any uniform data,
        // then we need to allocate a constant buffer to hold that data.
        //
        // TODO: Do we need to allocate a shadow copy for use from
        // the CPU?
        //
        // TODO: When/where do we bind this constant buffer into
        // a descriptor set for later use?
        //
        size_t uniformSize = layout->getElementTypeLayout()->getSize();
        if (uniformSize)
        {
            BufferResource::Desc bufferDesc;
            bufferDesc.init(uniformSize);
            bufferDesc.cpuAccessFlags |= Resource::AccessFlag::Write;
            SLANG_RETURN_ON_FAIL(renderer->createBufferResource(
                Resource::Usage::ConstantBuffer, bufferDesc, nullptr, m_buffer.writeRef()));
        }

#if 0
        // If the layout tells us there are any descriptor sets to
        // allocate, then we do so now.
        //
        for(auto descriptorSetInfo : layout->getDescriptorSets())
        {
            RefPtr<DescriptorSet> descriptorSet;
            SLANG_RETURN_ON_FAIL(renderer->createDescriptorSet(descriptorSetInfo->layout, descriptorSet.writeRef()));
            m_descriptorSets.add(descriptorSet);
        }
#endif

        m_resourceViews.setCount(layout->getResourceViewCount());
        m_samplers.setCount(layout->getSamplerCount());
        m_combinedTextureSamplers.setCount(layout->getCombinedTextureSamplerCount());

        // If the layout specifies that we have any sub-objects, then
        // we need to size the array to account for them.
        //
        Index subObjectCount = layout->getSubObjectCount();
        m_objects.setCount(subObjectCount);

        for (auto subObjectRangeInfo : layout->getSubObjectRanges())
        {
            RefPtr<GraphicsCommonShaderObjectLayout> subObjectLayout = subObjectRangeInfo.layout;

            // In the case where the sub-object range represents an
            // existential-type leaf field (e.g., an `IBar`), we
            // cannot pre-allocate the objet(s) to go into that
            // range, since we can't possibly know what to allocate
            // at this point.
            //
            if (!subObjectLayout)
                continue;
            //
            // Otherwise, we will allocate a sub-object to fill
            // in each entry in this range, based on the layout
            // information we already have.

            auto& bindingRangeInfo = layout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);
            for (Index i = 0; i < bindingRangeInfo.count; ++i)
            {
                RefPtr<GraphicsCommonShaderObject> subObject;
                SLANG_RETURN_ON_FAIL(
                    GraphicsCommonShaderObject::create(renderer, subObjectLayout, subObject.writeRef()));
                m_objects[bindingRangeInfo.baseIndex + i] = subObject;
            }
        }

        return SLANG_OK;
    }

    Result apply(
        IRenderer* renderer,
        PipelineType pipelineType,
        PipelineLayout* pipelineLayout,
        Index& ioRootIndex)
    {
        GraphicsCommonShaderObjectLayout* layout = m_layout;

        // Create the descritpor sets required by the layout...
        //
        List<RefPtr<DescriptorSet>> descriptorSets;
        for (auto descriptorSetInfo : layout->getDescriptorSets())
        {
            RefPtr<DescriptorSet> descriptorSet;
            SLANG_RETURN_ON_FAIL(
                renderer->createDescriptorSet(descriptorSetInfo->layout, descriptorSet.writeRef()));
            descriptorSets.add(descriptorSet);
        }

        SLANG_RETURN_ON_FAIL(_bindIntoDescriptorSets(descriptorSets.getBuffer()));

        for (auto descriptorSet : descriptorSets)
        {
            renderer->setDescriptorSet(pipelineType, pipelineLayout, ioRootIndex++, descriptorSet);
        }

        return SLANG_OK;
    }

    Result _bindIntoDescriptorSet(
        DescriptorSet* descriptorSet, Index baseRangeIndex, Index subObjectRangeArrayIndex)
    {
        GraphicsCommonShaderObjectLayout* layout = m_layout;

        if (m_buffer)
        {
            descriptorSet->setConstantBuffer(baseRangeIndex, subObjectRangeArrayIndex, m_buffer);
            baseRangeIndex++;
        }

        for (auto bindingRangeInfo : layout->getBindingRanges())
        {
            switch (bindingRangeInfo.bindingType)
            {
            case slang::BindingType::VaryingInput:
            case slang::BindingType::VaryingOutput:
                continue;

            default:
                break;
            }

            SLANG_ASSERT(bindingRangeInfo.descriptorSetIndex == 0);

            auto count = bindingRangeInfo.count;
            auto baseIndex = bindingRangeInfo.baseIndex;

            auto descriptorRangeIndex = baseRangeIndex + bindingRangeInfo.rangeIndexInDescriptorSet;
            auto descriptorArrayBaseIndex = subObjectRangeArrayIndex * count;

            switch (bindingRangeInfo.bindingType)
            {
            case slang::BindingType::ConstantBuffer:
            case slang::BindingType::ParameterBlock:
                break;

            case slang::BindingType::ExistentialValue:
                //
                // TODO: If the existential value is one that "fits" into the storage available,
                // then we should write its data directly into that area. Otherwise, we need
                // to bind its content as "pending" data which will come after any other data
                // beloning to the same set (that is, it's starting descriptorRangeIndex will
                // need to be one after the number of ranges accounted for in the original type)
                //
                break;

            case slang::BindingType::CombinedTextureSampler:
                for (Index i = 0; i < count; ++i)
                {
                    auto& slot = m_combinedTextureSamplers[baseIndex + i];
                    descriptorSet->setCombinedTextureSampler(
                        descriptorRangeIndex,
                        descriptorArrayBaseIndex + i,
                        slot.textureView,
                        slot.sampler);
                }
                break;

            case slang::BindingType::Sampler:
                for (Index i = 0; i < count; ++i)
                {
                    descriptorSet->setSampler(
                        descriptorRangeIndex,
                        descriptorArrayBaseIndex + i,
                        m_samplers[baseIndex + i]);
                }
                break;

            default:
                for (Index i = 0; i < count; ++i)
                {
                    descriptorSet->setResource(
                        descriptorRangeIndex,
                        descriptorArrayBaseIndex + i,
                        m_resourceViews[baseIndex + i]);
                }
                break;
            }
        }

        return SLANG_OK;
    }

public:
    virtual Result _bindIntoDescriptorSets(RefPtr<DescriptorSet>* descriptorSets)
    {
        GraphicsCommonShaderObjectLayout* layout = m_layout;

        if (m_buffer)
        {
            // TODO: look up binding infor for default constant buffer...
            descriptorSets[0]->setConstantBuffer(0, 0, m_buffer);
        }

        // Fill in the descriptor sets based on binding ranges
        //
        for (auto bindingRangeInfo : layout->getBindingRanges())
        {
            DescriptorSet* descriptorSet = descriptorSets[bindingRangeInfo.descriptorSetIndex];
            auto rangeIndex = bindingRangeInfo.rangeIndexInDescriptorSet;
            auto baseIndex = bindingRangeInfo.baseIndex;
            auto count = bindingRangeInfo.count;
            switch (bindingRangeInfo.bindingType)
            {
            case slang::BindingType::ConstantBuffer:
            case slang::BindingType::ParameterBlock:
                for (Index i = 0; i < count; ++i)
                {
                    GraphicsCommonShaderObject* subObject = m_objects[baseIndex + i];

                    subObject->_bindIntoDescriptorSet(descriptorSet, rangeIndex, i);
                }
                break;

            case slang::BindingType::CombinedTextureSampler:
                for (Index i = 0; i < count; ++i)
                {
                    auto& slot = m_combinedTextureSamplers[baseIndex + i];
                    descriptorSet->setCombinedTextureSampler(
                        rangeIndex, i, slot.textureView, slot.sampler);
                }
                break;

            case slang::BindingType::Sampler:
                for (Index i = 0; i < count; ++i)
                {
                    descriptorSet->setSampler(rangeIndex, i, m_samplers[baseIndex + i]);
                }
                break;

            case slang::BindingType::VaryingInput:
            case slang::BindingType::VaryingOutput:
                break;

            case slang::BindingType::ExistentialValue:
                // Here we are binding as if existential value is the same
                // as a constant buffer or parameter block, which will lead
                // to incorrect results...
                for (Index i = 0; i < count; ++i)
                {
                    GraphicsCommonShaderObject* subObject = m_objects[baseIndex + i];

                    subObject->_bindIntoDescriptorSet(descriptorSet, rangeIndex, i);
                }
                break;

            default:
                for (Index i = 0; i < count; ++i)
                {
                    descriptorSet->setResource(rangeIndex, i, m_resourceViews[baseIndex + i]);
                }
                break;
            }
        }
        return SLANG_OK;
    }

    RefPtr<GraphicsCommonShaderObjectLayout> m_layout = nullptr;
    RefPtr<BufferResource> m_buffer;

    List<RefPtr<ResourceView>> m_resourceViews;

    List<RefPtr<SamplerState>> m_samplers;

    struct CombinedTextureSamplerSlot
    {
        RefPtr<ResourceView> textureView;
        RefPtr<SamplerState> sampler;
    };
    List<CombinedTextureSamplerSlot> m_combinedTextureSamplers;

    //    List<RefPtr<DescriptorSet>> m_descriptorSets;
    List<RefPtr<GraphicsCommonShaderObject>> m_objects;
};

class EntryPointVars : public GraphicsCommonShaderObject
{
    typedef GraphicsCommonShaderObject Super;

public:
    static Result
    create(IRenderer* renderer, EntryPointLayout* layout, EntryPointVars** outShaderObject)
    {
        RefPtr<EntryPointVars> object = new EntryPointVars();
        SLANG_RETURN_ON_FAIL(object->init(renderer, layout));

        *outShaderObject = object.detach();
        return SLANG_OK;
    }

    EntryPointLayout* getLayout() { return static_cast<EntryPointLayout*>(m_layout.Ptr()); }

protected:
    Result init(IRenderer* renderer, EntryPointLayout* layout)
    {
        SLANG_RETURN_ON_FAIL(Super::init(renderer, layout));
        return SLANG_OK;
    }
};

class ProgramVars : public GraphicsCommonShaderObject
{
    typedef GraphicsCommonShaderObject Super;

public:
    static Result create(IRenderer* renderer, GraphicsCommonProgramLayout* layout, ProgramVars** outShaderObject)
    {
        RefPtr<ProgramVars> object = new ProgramVars();
        SLANG_RETURN_ON_FAIL(object->init(renderer, layout));

        *outShaderObject = object.detach();
        return SLANG_OK;
    }

    GraphicsCommonProgramLayout* getLayout() { return static_cast<GraphicsCommonProgramLayout*>(m_layout.Ptr()); }

    void apply(IRenderer* renderer, PipelineType pipelineType)
    {
        auto pipelineLayout = getLayout()->getPipelineLayout();

        Index rootIndex = 0;
        GraphicsCommonShaderObject::apply(renderer, pipelineType, pipelineLayout, rootIndex);

#if 0

        Index descriptorSetCount = m_descriptorSets.getCount();
        for(Index descriptorSetIndex = 0; descriptorSetIndex < descriptorSetCount; ++descriptorSetIndex)
        {
            renderer->setDescriptorSet(
                pipelineType,
                pipelineLayout,
                descriptorSetIndex,
                m_descriptorSets[descriptorSetIndex]);
        }
#endif

        // TODO: We also need to bind any descriptor sets that are
        // part of sub-objects of this object.
    }

    List<RefPtr<EntryPointVars>> const& getEntryPoints() const { return m_entryPoints; }

    
    Index getEntryPointCount() { return m_entryPoints.getCount(); }
    ShaderObject* getEntryPoint(Index index) { return m_entryPoints[index]; }


protected:
    virtual Result _bindIntoDescriptorSets(RefPtr<DescriptorSet>* descriptorSets)
    {
        SLANG_RETURN_ON_FAIL(Super::_bindIntoDescriptorSets(descriptorSets));

        auto entryPointCount = m_entryPoints.getCount();
        for (Index i = 0; i < entryPointCount; ++i)
        {
            auto entryPoint = m_entryPoints[i];
            auto& entryPointInfo = getLayout()->getEntryPoint(i);

            SLANG_RETURN_ON_FAIL(entryPoint->_bindIntoDescriptorSet(
                descriptorSets[0], entryPointInfo.rangeOffset, 0));
        }

        return SLANG_OK;
    }

    Result init(IRenderer* renderer, GraphicsCommonProgramLayout* layout)
    {
        SLANG_RETURN_ON_FAIL(Super::init(renderer, layout));

        for (auto entryPointInfo : layout->getEntryPoints())
        {
            RefPtr<EntryPointVars> entryPoint;
            SLANG_RETURN_ON_FAIL(
                EntryPointVars::create(renderer, entryPointInfo.layout, entryPoint.writeRef()));
            m_entryPoints.add(entryPoint);
        }

        return SLANG_OK;
    }

    List<RefPtr<EntryPointVars>> m_entryPoints;
};


Result GraphicsAPIRenderer::createShaderObjectLayout(
    slang::TypeLayoutReflection* typeLayout, ShaderObjectLayout** outLayout)
{
    RefPtr<GraphicsCommonShaderObjectLayout> layout;
    SLANG_RETURN_ON_FAIL(GraphicsCommonShaderObjectLayout::createForElementType(
        this, typeLayout, layout.writeRef()));
    *outLayout = layout.detach();
    return SLANG_OK;
}

Result GraphicsAPIRenderer::createShaderObject(ShaderObjectLayout* layout, ShaderObject** outObject)
{
    RefPtr<GraphicsCommonShaderObject> shaderObject;
    SLANG_RETURN_ON_FAIL(GraphicsCommonShaderObject::create(this,
        reinterpret_cast<GraphicsCommonShaderObjectLayout*>(layout), shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

Result GraphicsAPIRenderer::createRootShaderObject(
    ShaderObjectLayout* rootLayout, ShaderObject** outObject)
{
    RefPtr<ProgramVars> shaderObject;
    SLANG_RETURN_ON_FAIL(ProgramVars::create(this,
        dynamic_cast<GraphicsCommonProgramLayout*>(rootLayout),
        shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

Result GraphicsAPIRenderer::createRootShaderObjectLayout(
    slang::ProgramLayout* layout, ShaderObjectLayout** outLayout)
{
    RefPtr<GraphicsCommonProgramLayout> programLayout;
    auto slangReflection = layout;
    {
        GraphicsCommonProgramLayout::Builder builder(this);
        builder.addGlobalParams(slangReflection->getGlobalParamsVarLayout());

        // TODO: Also need to reflect entry points here.

        SlangInt entryPointCount = slangReflection->getEntryPointCount();
        for (SlangInt e = 0; e < entryPointCount; ++e)
        {
            auto slangEntryPoint = slangReflection->getEntryPointByIndex(e);

            EntryPointLayout::Builder entryPointBuilder(this);
            entryPointBuilder.addEntryPointParams(slangEntryPoint);

            RefPtr<EntryPointLayout> entryPointLayout;
            SLANG_RETURN_ON_FAIL(entryPointBuilder.build(entryPointLayout.writeRef()));

            builder.addEntryPoint(entryPointLayout);
        }

        SLANG_RETURN_ON_FAIL(builder.build(programLayout.writeRef()));
    }
    *outLayout = programLayout.detach();
    return SLANG_OK;
}

Result GraphicsAPIRenderer::bindRootShaderObject(PipelineType pipelineType, ShaderObject* object)
{
    auto programVars = dynamic_cast<ProgramVars*>(object);
    if (!programVars)
        return SLANG_E_INVALID_HANDLE;

    programVars->apply(this, pipelineType);
    return SLANG_OK;
}

void GraphicsAPIRenderer::preparePipelineDesc(GraphicsPipelineStateDesc& desc)
{
    if (desc.rootShaderObjectLayout)
    {
        auto rootLayout = dynamic_cast<GraphicsCommonProgramLayout*>(desc.rootShaderObjectLayout);
        desc.renderTargetCount = rootLayout->getRenderTargetCount();
        desc.pipelineLayout = rootLayout->getPipelineLayout();
    }
}

void GraphicsAPIRenderer::preparePipelineDesc(ComputePipelineStateDesc& desc)
{
    if (desc.rootShaderObjectLayout)
    {
        auto rootLayout = dynamic_cast<GraphicsCommonProgramLayout*>(desc.rootShaderObjectLayout);
        desc.pipelineLayout = rootLayout->getPipelineLayout();
    }
}

IRenderer* gfx::GraphicsAPIRenderer::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_IRenderer)
               ? static_cast<IRenderer*>(this)
               : nullptr;
}

}

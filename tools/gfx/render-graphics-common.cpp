#include "render-graphics-common.h"
#include "core/slang-basic.h"

using namespace Slang;

namespace gfx
{

class GraphicsCommonShaderObjectLayout : public ShaderObjectLayoutBase
{
public:
    struct BindingRangeInfo
    {
        slang::BindingType bindingType;
        Index count;
        Index baseIndex;
        Index descriptorSetIndex;
        Index rangeIndexInDescriptorSet;

        // Returns true if this binding range consumes a specialization argument slot.
        bool isSpecializationArg() const
        {
            return bindingType == slang::BindingType::ExistentialValue;
        }
    };

    struct SubObjectRangeInfo
    {
        RefPtr<GraphicsCommonShaderObjectLayout> layout;
        Index bindingRangeIndex;
    };

    struct DescriptorSetInfo : public RefObject
    {
        ComPtr<IDescriptorSetLayout> layout;
        Slang::Int space = -1;
    };

    struct Builder
    {
    public:
        Builder(RendererBase* renderer)
            : m_renderer(renderer)
        {}

        RendererBase* m_renderer;
        slang::TypeLayoutReflection* m_elementTypeLayout;

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
            List<IDescriptorSetLayout::SlotRangeDesc> slotRangeDescs;
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
                CASE(RawBuffer, ReadOnlyStorageBuffer);
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

                    switch (slangBindingType)
                    {
                    case slang::BindingType::ExistentialValue:
                    case slang::BindingType::InlineUniformData:
                        continue;
                    default:
                        break;
                    }

                    auto gfxDescriptorType = _mapDescriptorType(slangBindingType);

                    IDescriptorSetLayout::SlotRangeDesc descriptorRangeDesc;
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
            auto layout =
                RefPtr<GraphicsCommonShaderObjectLayout>(new GraphicsCommonShaderObjectLayout());
            SLANG_RETURN_ON_FAIL(layout->_init(this));

            *outLayout = layout.detach();
            return SLANG_OK;
        }
    };

    static Result createForElementType(
        RendererBase* renderer,
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

    RendererBase* getRenderer() { return m_renderer; }

    slang::TypeReflection* getType()
    {
        return m_elementTypeLayout->getType();
    }
protected:
    Result _init(Builder const* builder)
    {
        auto renderer = builder->m_renderer;

        initBase(renderer, builder->m_elementTypeLayout);

        m_bindingRanges = builder->m_bindingRanges;

        for (auto descriptorSetBuildInfo : builder->m_descriptorSetBuildInfos)
        {
            auto& slotRangeDescs = descriptorSetBuildInfo->slotRangeDescs;
            IDescriptorSetLayout::Desc desc;
            desc.slotRangeCount = slotRangeDescs.getCount();
            desc.slotRanges = slotRangeDescs.getBuffer();

            ComPtr<IDescriptorSetLayout> descriptorSetLayout;
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

    List<RefPtr<DescriptorSetInfo>> m_descriptorSets;
    List<BindingRangeInfo> m_bindingRanges;
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
            : Super::Builder(static_cast<RendererBase*>(renderer))
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
            : Super::Builder(static_cast<RendererBase*>(renderer))
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
            else
            {
                info.rangeOffset = 0;
            }

            auto slangEntryPointLayout = entryPointLayout->getSlangLayout();
            _addDescriptorSets(
                slangEntryPointLayout->getTypeLayout(), slangEntryPointLayout->getVarLayout());

            m_entryPoints.add(info);
        }

        List<EntryPointInfo> m_entryPoints;
    };

    Slang::Int getRenderTargetCount() { return m_renderTargetCount; }

    IPipelineLayout* getPipelineLayout() { return m_pipelineLayout; }

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

        List<IPipelineLayout::DescriptorSetDesc> pipelineDescriptorSets;
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

        IPipelineLayout::Desc pipelineLayoutDesc;

        // HACK: we set `renderTargetCount` to zero here becasue otherwise the D3D12
        // render back-end will adjust all UAV registers by this value to account
        // for the `SV_Target<N>` outputs implicitly consuming `u<N>` registers for
        // Shader Model 5.0.
        //
        // When using the shader object path, all registers are being set via Slang
        // reflection information, and we do not need/want the automatic adjustment.
        //
        // TODO: Once we eliminate the non-shader-object path, this whole issue should
        // be moot, because the `ProgramLayout` should own/be the pipeline layout anyway.
        //
        pipelineLayoutDesc.renderTargetCount = 0;

        pipelineLayoutDesc.descriptorSetCount = pipelineDescriptorSets.getCount();
        pipelineLayoutDesc.descriptorSets = pipelineDescriptorSets.getBuffer();

        SLANG_RETURN_ON_FAIL(
            renderer->createPipelineLayout(pipelineLayoutDesc, m_pipelineLayout.writeRef()));

        return SLANG_OK;
    }

    static void _addDescriptorSetsRec(
        GraphicsCommonShaderObjectLayout* layout,
        List<IPipelineLayout::DescriptorSetDesc>& ioPipelineDescriptorSets)
    {
        for (auto descriptorSetInfo : layout->getDescriptorSets())
        {
            IPipelineLayout::DescriptorSetDesc pipelineDescriptorSet;
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

    ComPtr<IPipelineLayout> m_pipelineLayout;
};

class GraphicsCommonShaderObject : public ShaderObjectBase
{
public:
    static Result create(
        IRenderer* renderer,
        GraphicsCommonShaderObjectLayout* layout,
        GraphicsCommonShaderObject** outShaderObject)
    {
        auto object = ComPtr<GraphicsCommonShaderObject>(new GraphicsCommonShaderObject());
        SLANG_RETURN_ON_FAIL(object->init(renderer, layout));

        *outShaderObject = object.detach();
        return SLANG_OK;
    }

    RendererBase* getRenderer() { return m_layout->getRenderer(); }

    SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return 0; }

    SLANG_NO_THROW Result SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint)
        SLANG_OVERRIDE
    {
        *outEntryPoint = nullptr;
        return SLANG_OK;
    }

    GraphicsCommonShaderObjectLayout* getLayout()
    {
        return static_cast<GraphicsCommonShaderObjectLayout*>(m_layout.Ptr());
    }

    SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() SLANG_OVERRIDE
    {
        return m_layout->getElementTypeLayout();
    }

    SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& inOffset, void const* data, size_t inSize) SLANG_OVERRIDE
    {
        Index offset = inOffset.uniformOffset;
        Index size = inSize;

        char* dest = m_ordinaryData.getBuffer();
        Index availableSize = m_ordinaryData.getCount();

        // TODO: We really should bounds-check access rather than silently ignoring sets
        // that are too large, but we have several test cases that set more data than
        // an object actually stores on several targets...
        //
        if(offset < 0)
        {
            size += offset;
            offset = 0;
        }
        if((offset + size) >= availableSize)
        {
            size = availableSize - offset;
        }

        memcpy(dest + offset, data, size);

        return SLANG_OK;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setObject(ShaderOffset const& offset, IShaderObject* object)
        SLANG_OVERRIDE
    {
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        auto layout = getLayout();
        if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;

        auto subObject = static_cast<GraphicsCommonShaderObject*>(object);

        auto bindingRangeIndex = offset.bindingRangeIndex;
        auto& bindingRange = layout->getBindingRange(bindingRangeIndex);

        m_objects[bindingRange.baseIndex + offset.bindingArrayIndex] = subObject;

        // If the range being assigned into represents an interface/existential-type leaf field,
        // then we need to consider how the `object` being assigned here affects specialization.
        // We may also need to assign some data from the sub-object into the ordinary data
        // buffer for the parent object.
        //
        if( bindingRange.bindingType == slang::BindingType::ExistentialValue )
        {
            // A leaf field of interface type is laid out inside of the parent object
            // as a tuple of `(RTTI, WitnessTable, Payload)`. The layout of these fields
            // is a contract between the compiler and any runtime system, so we will
            // need to rely on details of the binary layout.

            // The first field of the tuple (offset zero) is the run-time type information (RTTI)
            // ID for the concrete type being stored into the field.
            //
            // TODO: We need to be able to gather the RTTI type ID from `object` and then
            // use `setData(offset, &TypeID, sizeof(TypeID))`.

            // The second field of the tuple (offset 8) is the ID of the "witness" for the
            // conformance of the concrete type to the interface used by this field.
            //
            // The concrete type doing the conforming will come from `object`, while the type
            // being conformed to will be the leaf type of the field, which can be queried
            // as a type layout from the Slang reflection information:
            //
            auto leafTypeLayout = layout->getElementTypeLayout()->getBindingRangeLeafTypeLayout(bindingRangeIndex);
            //
            // TODO: In order to set the witness-table field here we would need to:
            //
            // * Look up the conformance witness using the Slang refelction API. We probably
            //   need to cache those lookups.
            //
            // * Query the Slang reflection API for the ID of that conformance. This probably
            //   also needs caching.
            //
            // * Write the ID of the witness using `setData(offset+8, &WitnessID, sizeof(WitnessID))`

            // The third field of the tuple (offset 16) is the "payload" that is supposed to
            // hold the data for a value of the given concrete type.
            //
            // There are two cases we need to consider here for how the payload might be used:
            //
            // * If the concrete type of the value being bound is one that can "fit" into the
            //   available payload space,  then it should be stored in the payload.
            //
            // * If the concrete type of the value cannot fit in the payload space, then it
            //   will need to be stored somewhere else.
            //
            if(_doesValueFitInExistentialPayload(subObject, leafTypeLayout))
            {
                // If the value can fit in the payload area, then we will go ahead and copy
                // its bytes into that area.
                //
                auto payloadOffset = offset;
                payloadOffset.uniformOffset += 16;
                setData(payloadOffset, subObject->m_ordinaryData.getBuffer(), subObject->m_ordinaryData.getCount());
            }
            else
            {
                // If the value does *not *fit in the payload area, then there is nothing
                // we can do at this point (beyond saving a reference to the sub-object, which
                // was handled above).
                //
                // Once all the sub-objects have been set into the parent object, we can
                // compute a specialized layout for it, and that specialized layout can tell
                // us where the data for these sub-objects has been laid out.
            }
        }

        return SLANG_E_NOT_IMPLEMENTED;
    }

    bool _doesValueFitInExistentialPayload(
        GraphicsCommonShaderObject*     object,
        slang::TypeLayoutReflection*    existentialFieldLayout)
    {
        // Our task here is to figure out if the value of `object` can fit
        // into an existential-type leaf field defined by `existentialFieldLayout`.

        // We can start by asking how many bytes the concrete type of the object consumes.
        //
        auto concreteTypeLayout = object->getElementTypeLayout();
        auto concreteValueSize = concreteTypeLayout->getSize();

        // We can also compute how many bytes the existential-type field consumes,
        // but we need to remember that the *payload* part of that field comes after
        // the header with RTTI and witness-table IDs, so the payload is 16 bytes
        // smaller than the entire field.
        //
        auto existentialValueSize = existentialFieldLayout->getSize();
        auto existentialPayloadSize = existentialValueSize - 16;

        // If the concrete type consumes more ordinary bytes than we have in the payload,
        // it cannot possibly fit.
        //
        if(concreteValueSize > existentialPayloadSize)
            return false;

        // It is possible that the ordinary bytes of `concreteTypeLayout` can fit
        // in the payload, but that type might also use storage other than ordinary
        // bytes. In that case, the value would *not* fit, because all the non-ordinary
        // data can't fit in the payload at all.
        //
        auto categoryCount = concreteTypeLayout->getCategoryCount();
        for(unsigned int i = 0; i < categoryCount; ++i)
        {
            auto category = concreteTypeLayout->getCategoryByIndex(i);
            switch(category)
            {
            // We want to ignore any ordinary/uniform data usage, since that
            // was already checked above.
            //
            case slang::ParameterCategory::Uniform:
                break;

            // Any other kind of data consumed means the value cannot possibly fit.
            default:
                return false;

            // TODO: Are there any cases of resource usage that need to be ignored here?
            // E.g., if the sub-object contains its own existential-type fields (which
            // get reflected as consuming "existential value" storage) should that be
            // ignored?
            }
        }

        // If we didn't reject the concrete type above for either its ordinary
        // data or some use of non-ordinary data, then it seems like it must fit.
        //
        return true;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getObject(ShaderOffset const& offset, IShaderObject** outObject)
        SLANG_OVERRIDE
    {
        SLANG_ASSERT(outObject);
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        auto layout = getLayout();
        if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

        auto object = m_objects[bindingRange.baseIndex + offset.bindingArrayIndex].Ptr();
        object->addRef();
        *outObject = object;

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

    SLANG_NO_THROW Result SLANG_MCALL
        setResource(ShaderOffset const& offset, IResourceView* resourceView) SLANG_OVERRIDE
    {
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        auto layout = getLayout();
        if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

        m_resourceViews[bindingRange.baseIndex + offset.bindingArrayIndex] = resourceView;
        return SLANG_OK;
    }

    SLANG_NO_THROW Result SLANG_MCALL setSampler(ShaderOffset const& offset, ISamplerState* sampler)
        SLANG_OVERRIDE
    {
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        auto layout = getLayout();
        if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

        m_samplers[bindingRange.baseIndex + offset.bindingArrayIndex] = sampler;
        return SLANG_OK;
    }

    SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
        ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) SLANG_OVERRIDE
    {
        if (offset.bindingRangeIndex < 0)
            return SLANG_E_INVALID_ARG;
        auto layout = getLayout();
        if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
            return SLANG_E_INVALID_ARG;
        auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

        auto& slot = m_combinedTextureSamplers[bindingRange.baseIndex + offset.bindingArrayIndex];
        slot.textureView = textureView;
        slot.sampler = sampler;
        return SLANG_OK;
    }

public:
    // Appends all types that are used to specialize the element type of this shader object in `args` list.
    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
    {
        auto& subObjectRanges = getLayout()->getSubObjectRanges();
        // The following logic is built on the assumption that all fields that involve existential types (and
        // therefore require specialization) will results in a sub-object range in the type layout.
        // This allows us to simply scan the sub-object ranges to find out all specialization arguments.
        for (Index subObjIndex = 0; subObjIndex < subObjectRanges.getCount(); subObjIndex++)
        {
            // Retrieve the corresponding binding range of the sub object.
            auto bindingRange = getLayout()->getBindingRange(subObjectRanges[subObjIndex].bindingRangeIndex);
            switch (bindingRange.bindingType)
            {
            case slang::BindingType::ExistentialValue:
            {
                // A binding type of `ExistentialValue` means the sub-object represents a interface-typed field.
                // In this case the specialization argument for this field is the actual specialized type of the bound
                // shader object. If the shader object's type is an ordinary type without existential fields, then the
                // type argument will simply be the ordinary type. But if the sub object's type is itself a specialized
                // type, we need to make sure to use that type as the specialization argument.

                // TODO: need to implement the case where the field is an array of existential values.
                SLANG_ASSERT(bindingRange.count == 1);
                ExtendedShaderObjectType specializedSubObjType;
                SLANG_RETURN_ON_FAIL(m_objects[subObjIndex]->getSpecializedShaderObjectType(&specializedSubObjType));
                args.add(specializedSubObjType);
                break;
            }
            case slang::BindingType::ParameterBlock:
            case slang::BindingType::ConstantBuffer:
                // Currently we only handle the case where the field's type is
                // `ParameterBlock<SomeStruct>` or `ConstantBuffer<SomeStruct>`, where `SomeStruct` is a struct type
                // (not directly an interface type). In this case, we just recursively collect the specialization arguments
                // from the bound sub object.
                SLANG_RETURN_ON_FAIL(m_objects[subObjIndex]->collectSpecializationArgs(args));
                // TODO: we need to handle the case where the field is of the form `ParameterBlock<IFoo>`. We should treat
                // this case the same way as the `ExistentialValue` case here, but currently we lack a mechanism to distinguish
                // the two scenarios.
                break;
            }
            // TODO: need to handle another case where specialization happens on resources fields e.g. `StructuredBuffer<IFoo>`.
        }
        return SLANG_OK;
    }


protected:
    friend class ProgramVars;

    Result init(IRenderer* renderer, GraphicsCommonShaderObjectLayout* layout)
    {
        m_layout = layout;

        // If the layout tells us that there is any uniform data,
        // then we will allocate a CPU memory buffer to hold that data
        // while it is being set from the host.
        //
        // Once the user is done setting the parameters/fields of this
        // shader object, we will produce a GPU-memory version of the
        // uniform data (which includes values from this object and
        // any existential-type sub-objects).
        //
        size_t uniformSize = layout->getElementTypeLayout()->getSize();
        if (uniformSize)
        {
            m_ordinaryData.setCount(uniformSize);
            memset(m_ordinaryData.getBuffer(), 0, uniformSize);
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
            auto subObjectLayout = subObjectRangeInfo.layout;

            // In the case where the sub-object range represents an
            // existential-type leaf field (e.g., an `IBar`), we
            // cannot pre-allocate the object(s) to go into that
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
        IPipelineLayout* pipelineLayout,
        Index& ioRootIndex)
    {
        GraphicsCommonShaderObjectLayout* layout = getLayout();

        // Create the descritpor sets required by the layout...
        //
        List<ComPtr<IDescriptorSet>> descriptorSets;
        for (auto descriptorSetInfo : layout->getDescriptorSets())
        {
            ComPtr<IDescriptorSet> descriptorSet;
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

        /// Write the uniform/ordinary data of this object into the given `dest` buffer at the given `offset`
    Result _writeOrdinaryData(
        char*                               dest,
        size_t                              destSize,
        GraphicsCommonShaderObjectLayout*   specializedLayout)
    {
        auto src = m_ordinaryData.getBuffer();
        auto srcSize = size_t(m_ordinaryData.getCount());

        SLANG_ASSERT(srcSize <= destSize);

        memcpy(dest, src, srcSize);

        // In the case where this object has any sub-objects of
        // existential/interface type, we need to recurse on those objects
        // that need to write their state into an appropriate "pending" allocation.
        //
        // Note: Any values that could fit into the "payload" included
        // in the existential-type field itself will have already been
        // written as part of `setObject()`. This loop only needs to handle
        // those sub-objects that do not "fit."
        //
        // An implementers looking at this code might wonder if things could be changed
        // so that *all* writes related to sub-objects for interface-type fields could
        // be handled in this one location, rather than having some in `setObject()` and
        // others handled here.
        //
        Index subObjectRangeCounter;
        for( auto const& subObjectRangeInfo : specializedLayout->getSubObjectRanges() )
        {
            Index subObjectRangeIndex = subObjectRangeCounter++;
            auto const& bindingRangeInfo = specializedLayout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);

            // We only need to handle sub-object ranges for interface/existential-type fields,
            // because fields of constant-buffer or parameter-block type are responsible for
            // the ordinary/uniform data of their own existential/interface-type sub-objects.
            //
            if(bindingRangeInfo.bindingType != slang::BindingType::ExistentialValue)
                continue;

            // Each sub-object range represents a single "leaf" field, but might be nested
            // under zero or more outer arrays, such that the number of existential values
            // in the same range can be one or more.
            //
            auto count = bindingRangeInfo.count;

            // We are not concerned with the case where the existential value(s) in the range
            // git into the payload part of the leaf field.
            //
            // In the case where the value didn't fit, the Slang layout strategy would have
            // considered the requirements of the value as a "pending" allocation, and would
            // allocate storage for the ordinary/uniform part of that pending allocation inside
            // of the parent object's type layout.
            //
            // Here we assume that the Slang reflection API can provide us with a single byte
            // offset and stride for the location of the pending data allocation in the specialized
            // type layout, which will store the values for this sub-object range.
            //
            // TODO: The reflection API functions we are assuming here haven't been implemented
            // yet, so the functions being called here are stubs.
            //
            // TODO: It might not be that a single sub-object range can reliably map to a single
            // contiguous array with a single stride; we need to carefully consider what the layout
            // logic does for complex cases with multiple layers of nested arrays and structures.
            //
            size_t subObjectRangePendingDataOffset = _getSubObjectRangePendingDataOffset(specializedLayout, subObjectRangeIndex);
            size_t subObjectRangePendingDataStride = _getSubObjectRangePendingDataStride(specializedLayout, subObjectRangeIndex);

            // If the range doesn't actually need/use the "pending" allocation at all, then
            // we need to detect that case and skip such ranges.
            //
            // TODO: This should probably be handled on a per-object basis by caching a "does it fit?"
            // bit as part of the information for bound sub-objects, given that we already
            // compute the "does it fit?" status as part of `setObject()`.
            //
            if(subObjectRangePendingDataOffset == 0)
                continue;

            for( Slang::Index i = 0; i < count; ++i )
            {
                auto subObject = m_objects[bindingRangeInfo.baseIndex + i];

                RefPtr<GraphicsCommonShaderObjectLayout> subObjectLayout;
                SLANG_RETURN_ON_FAIL(subObject->_getSpecializedLayout(subObjectLayout.writeRef()));

                auto subObjectOffset = subObjectRangePendingDataOffset + i*subObjectRangePendingDataStride;

                subObject->_writeOrdinaryData(dest + subObjectOffset, destSize - subObjectOffset, subObjectLayout);
            }
        }

        return SLANG_OK;
    }

    // As discussed in `_writeOrdinaryData()`, these methods are just stubs waiting for
    // the "flat" Slang refelction information to provide access to the relevant data.
    //
    size_t _getSubObjectRangePendingDataOffset(GraphicsCommonShaderObjectLayout* specializedLayout, Index subObjectRangeIndex) { return 0; }
    size_t _getSubObjectRangePendingDataStride(GraphicsCommonShaderObjectLayout* specializedLayout, Index subObjectRangeIndex) { return 0; }

        /// Ensure that the `m_ordinaryDataBuffer` has been created, if it is needed
    Result _ensureOrdinaryDataBufferCreatedIfNeeded()
    {
        // If we have already created a buffer to hold ordinary data, then we should
        // simply re-use that buffer rather than re-create it.
        //
        // TODO: Simply re-using the buffer without any kind of validation checks
        // means that we are assuming that users cannot or will not perform any `set`
        // operations on a shader object once an operation has requested this buffer
        // be created. We need to enforce that rule if we want to rely on it.
        //
        if( m_ordinaryDataBuffer )
            return SLANG_OK;

        // Computing the size of the ordinary data buffer is *not* just as simple
        // as using the size of the `m_ordinayData` array that we store. The reason
        // for the added complexity is that interface-type fields may lead to the
        // storage being specialized such that it needs extra appended data to
        // store the concrete values that logically belong in those interface-type
        // fields but wouldn't fit in the fixed-size allocation we gave them.
        //
        // TODO: We need to actually implement that logic by using reflection
        // data computed for the specialized type of this shader object.
        // For now we just make the simple assumption described above despite
        // knowing that it is false.
        //
        RefPtr<GraphicsCommonShaderObjectLayout> specializedLayout;
        SLANG_RETURN_ON_FAIL(_getSpecializedLayout(specializedLayout.writeRef()));

        auto specializedOrdinaryDataSize = specializedLayout->getElementTypeLayout()->getSize();
        if(specializedOrdinaryDataSize == 0)
            return SLANG_OK;

        // Once we have computed how large the buffer should be, we can allocate
        // it using the existing public `IRenderer` API.
        //
        IRenderer* renderer = getRenderer();
        IBufferResource::Desc bufferDesc;
        bufferDesc.init(specializedOrdinaryDataSize);
        bufferDesc.cpuAccessFlags |= IResource::AccessFlag::Write;
        SLANG_RETURN_ON_FAIL(renderer->createBufferResource(
            IResource::Usage::ConstantBuffer, bufferDesc, nullptr, m_ordinaryDataBuffer.writeRef()));

        // Once the buffer is allocated, we can use `_writeOrdinaryData` to fill it in.
        //
        // Note that `_writeOrdinaryData` is potentially recursive in the case
        // where this object contains interface/existential-type fields, so we
        // don't need or want to inline it into this call site.
        //
        char* dest = (char*)renderer->map(m_ordinaryDataBuffer, MapFlavor::HostWrite);
        SLANG_RETURN_ON_FAIL(_writeOrdinaryData(dest, specializedOrdinaryDataSize, specializedLayout));
        renderer->unmap(m_ordinaryDataBuffer);

        return SLANG_OK;
    }

        /// Bind the buffer for ordinary/uniform data, if needed
    Result _bindOrdinaryDataBufferIfNeeded(IDescriptorSet* descriptorSet, Index* ioBaseRangeIndex, Index subObjectRangeArrayIndex)
    {
        // We are going to need to tweak the base binding range index
        // used for descriptor-set writes if and only if we actually
        // bind a buffer for ordinary data.
        //
        auto& baseRangeIndex = *ioBaseRangeIndex;

        // We start by ensuring that the buffer is created, if it is needed.
        //
        SLANG_RETURN_ON_FAIL(_ensureOrdinaryDataBufferCreatedIfNeeded());

        // If we did indeed need/create a buffer, then we must bind it into
        // the given `descriptorSet` and update the base range index for
        // subsequent binding operations to account for it.
        //
        if (m_ordinaryDataBuffer)
        {
            descriptorSet->setConstantBuffer(baseRangeIndex, subObjectRangeArrayIndex, m_ordinaryDataBuffer);
            baseRangeIndex++;
        }

        return SLANG_OK;
    }

    Result _bindIntoDescriptorSet(
        IDescriptorSet* descriptorSet, Index baseRangeIndex, Index subObjectRangeArrayIndex)
    {
        GraphicsCommonShaderObjectLayout* layout = getLayout();

        _bindOrdinaryDataBufferIfNeeded(descriptorSet, &baseRangeIndex, subObjectRangeArrayIndex);

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
    virtual Result _bindIntoDescriptorSets(ComPtr<IDescriptorSet>* descriptorSets)
    {
        GraphicsCommonShaderObjectLayout* layout = getLayout();

        Index baseRangeIndex = 0;
        _bindOrdinaryDataBufferIfNeeded(descriptorSets[0], &baseRangeIndex, 0);

        // Fill in the descriptor sets based on binding ranges
        //
        for (auto bindingRangeInfo : layout->getBindingRanges())
        {
            auto descriptorSet = descriptorSets[bindingRangeInfo.descriptorSetIndex];
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

        /// Any "ordinary" / uniform data for this object
    List<char> m_ordinaryData;

    List<ComPtr<IResourceView>> m_resourceViews;

    List<ComPtr<ISamplerState>> m_samplers;

    struct CombinedTextureSamplerSlot
    {
        ComPtr<IResourceView> textureView;
        ComPtr<ISamplerState> sampler;
    };
    List<CombinedTextureSamplerSlot> m_combinedTextureSamplers;

    List<RefPtr<GraphicsCommonShaderObject>> m_objects;

        /// A constant buffer used to stored ordinary data for this object
        /// and existential-type sub-objects.
        ///
        /// Created on demand with `_createOrdinaryDataBufferIfNeeded()`
    ComPtr<IBufferResource> m_ordinaryDataBuffer;


    Result _getSpecializedLayout(GraphicsCommonShaderObjectLayout** outLayout)
    {
        if(!m_specializedLayout)
        {
            ExtendedShaderObjectType extendedType;
            SLANG_RETURN_ON_FAIL(getSpecializedShaderObjectType(&extendedType));

            auto renderer = getRenderer();
            RefPtr<ShaderObjectLayoutBase> layout;
            SLANG_RETURN_ON_FAIL(renderer->getShaderObjectLayout(extendedType.slangType, layout.writeRef()));

            m_specializedLayout = static_cast<GraphicsCommonShaderObjectLayout*>(layout.detach());
        }
        *outLayout = RefPtr<GraphicsCommonShaderObjectLayout>(m_specializedLayout).detach();
        return SLANG_OK;
    }

    RefPtr<GraphicsCommonShaderObjectLayout> m_specializedLayout;
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

    
    UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return (UInt)m_entryPoints.getCount(); }
    SlangResult SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint) SLANG_OVERRIDE
    {
        *outEntryPoint = m_entryPoints[index];
        m_entryPoints[index]->addRef();
        return SLANG_OK;
    }

    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
    {
        SLANG_RETURN_ON_FAIL(GraphicsCommonShaderObject::collectSpecializationArgs(args));
        for (auto& entryPoint : m_entryPoints)
        {
            SLANG_RETURN_ON_FAIL(entryPoint->collectSpecializationArgs(args));
        }
        return SLANG_OK;
    }

protected:
    virtual Result _bindIntoDescriptorSets(ComPtr<IDescriptorSet>* descriptorSets) override
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
    slang::TypeLayoutReflection* typeLayout,
    ShaderObjectLayoutBase** outLayout)
{
    RefPtr<GraphicsCommonShaderObjectLayout> layout;
    SLANG_RETURN_ON_FAIL(GraphicsCommonShaderObjectLayout::createForElementType(
        this, typeLayout, layout.writeRef()));
    *outLayout = layout.detach();
    return SLANG_OK;
}

Result GraphicsAPIRenderer::createShaderObject(
    ShaderObjectLayoutBase* layout,
    IShaderObject** outObject)
{
    RefPtr<GraphicsCommonShaderObject> shaderObject;
    SLANG_RETURN_ON_FAIL(GraphicsCommonShaderObject::create(this,
        reinterpret_cast<GraphicsCommonShaderObjectLayout*>(layout), shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

Result SLANG_MCALL GraphicsAPIRenderer::createRootShaderObject(
    IShaderProgram* program,
    IShaderObject** outObject)
{
    auto commonProgram = dynamic_cast<GraphicsCommonShaderProgram*>(program);

    RefPtr<ProgramVars> shaderObject;
    SLANG_RETURN_ON_FAIL(ProgramVars::create(this,
        commonProgram->getLayout(),
        shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

Result GraphicsAPIRenderer::initProgramCommon(
    GraphicsCommonShaderProgram*    program,
    IShaderProgram::Desc const&     desc)
{
    auto slangProgram = desc.slangProgram;
    if(!slangProgram)
        return SLANG_OK;

    auto slangReflection = slangProgram->getLayout(0);
    if(!slangReflection)
        return SLANG_FAIL;

    RefPtr<GraphicsCommonProgramLayout> programLayout;
    {
        GraphicsCommonProgramLayout::Builder builder(this);
        builder.addGlobalParams(slangReflection->getGlobalParamsVarLayout());

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
    program->slangProgram = slangProgram;
    program->m_layout = programLayout;

    return SLANG_OK;
}

Result SLANG_MCALL
    GraphicsAPIRenderer::bindRootShaderObject(PipelineType pipelineType, IShaderObject* object)
{
    auto programVars = dynamic_cast<ProgramVars*>(object);
    if (!programVars)
        return SLANG_E_INVALID_HANDLE;

    SLANG_RETURN_ON_FAIL(maybeSpecializePipeline(programVars));

    // Apply shader parameter bindings.
    programVars->apply(this, pipelineType);
    return SLANG_OK;
}

GraphicsCommonProgramLayout* gfx::GraphicsCommonShaderProgram::getLayout() const
{
    return static_cast<GraphicsCommonProgramLayout*>(m_layout.Ptr());
}

void GraphicsAPIRenderer::preparePipelineDesc(GraphicsPipelineStateDesc& desc)
{
    if (!desc.pipelineLayout)
    {
        auto program = dynamic_cast<GraphicsCommonShaderProgram*>(desc.program);
        auto rootLayout = program->getLayout();
        desc.renderTargetCount = rootLayout->getRenderTargetCount();
        desc.pipelineLayout = rootLayout->getPipelineLayout();
    }
}

void GraphicsAPIRenderer::preparePipelineDesc(ComputePipelineStateDesc& desc)
{
    if (!desc.pipelineLayout)
    {
        auto program = dynamic_cast<GraphicsCommonShaderProgram*>(desc.program);
        auto rootLayout = program->getLayout();
        desc.pipelineLayout = rootLayout->getPipelineLayout();
    }
}

}

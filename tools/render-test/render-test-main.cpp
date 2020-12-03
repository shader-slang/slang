// render-test-main.cpp

#define _CRT_SECURE_NO_WARNINGS 1

#include "options.h"
#include "render.h"

#include "slang-support.h"
#include "surface.h"
#include "png-serialize-util.h"

#include "shader-renderer-util.h"

#include "../source/core/slang-io.h"
#include "../source/core/slang-string-util.h"

#include "core/slang-token-reader.h"

#include "shader-input-layout.h"
#include <stdio.h>
#include <stdlib.h>

#include "window.h"

#include "../../source/core/slang-test-tool-util.h"

#include "cpu-compute-util.h"

#if RENDER_TEST_CUDA
#   include "cuda/cuda-compute-util.h"
#endif

namespace renderer_test {

using Slang::Result;

int gWindowWidth = 1024;
int gWindowHeight = 768;

//
// For the purposes of a small example, we will define the vertex data for a
// single triangle directly in the source file. It should be easy to extend
// this example to load data from an external source, if desired.
//

struct Vertex
{
    float position[3];
    float color[3];
    float uv[2];
};

static const Vertex kVertexData[] =
{
    { { 0,  0, 0.5 }, {1, 0, 0} , {0, 0} },
    { { 0,  1, 0.5 }, {0, 0, 1} , {1, 0} },
    { { 1,  0, 0.5 }, {0, 1, 0} , {1, 1} },
};
static const int kVertexCount = SLANG_COUNT_OF(kVertexData);

using namespace Slang;

static void _outputProfileTime(uint64_t startTicks, uint64_t endTicks)
{
    WriterHelper out = StdWriters::getOut();
    double time = double(endTicks - startTicks) / ProcessUtil::getClockFrequency();
    out.print("profile-time=%g\n", time);
}

class ProgramVars;

struct ShaderOutputPlan
{
    struct Item
    {
        Index               inputLayoutEntryIndex;
        RefPtr<Resource>    resource;
    };

    List<Item> items;
};

class RenderTestApp : public WindowListener
{
public:
    // WindowListener
    virtual Result update(Window* window) SLANG_OVERRIDE;

    // At initialization time, we are going to load and compile our Slang shader
    // code, and then create the API objects we need for rendering.
    virtual Result initialize(
        SlangSession* session,
        Renderer* renderer,
        const Options& options,
        const ShaderCompilerUtil::Input& input) = 0;
    void runCompute();
    void renderFrame();
    void finalize();

    virtual void applyBinding(PipelineType pipelineType) = 0;
    virtual void setProjectionMatrix() = 0;
    virtual Result writeBindingOutput(BindRoot* bindRoot, const char* fileName) = 0;

    Result writeScreen(const char* filename);

protected:
    /// Called in initialize
    Result _initializeShaders(
        SlangSession* session,
        Renderer* renderer,
        Options::ShaderProgramType shaderType,
        const ShaderCompilerUtil::Input& input);

    uint64_t m_startTicks;

    // variables for state to be used for rendering...
    uintptr_t m_constantBufferSize;

    RefPtr<Renderer> m_renderer;

    RefPtr<InputLayout> m_inputLayout;
    RefPtr<BufferResource> m_vertexBuffer;
    RefPtr<ShaderProgram> m_shaderProgram;
    RefPtr<PipelineState> m_pipelineState;

    ShaderCompilerUtil::OutputAndLayout m_compilationOutput;

    ShaderInputLayout m_shaderInputLayout; ///< The binding layout

    Options m_options;
};

class LegacyRenderTestApp : public RenderTestApp
{
public:
    virtual void applyBinding(PipelineType pipelineType) SLANG_OVERRIDE;
    virtual void setProjectionMatrix() SLANG_OVERRIDE;
    virtual Result initialize(
        SlangSession* session,
        Renderer* renderer,
        const Options& options,
        const ShaderCompilerUtil::Input& input) SLANG_OVERRIDE;

    BindingStateImpl* getBindingState() const { return m_bindingState; }

    virtual Result writeBindingOutput(BindRoot* bindRoot, const char* fileName) override;

protected:
	uintptr_t m_constantBufferSize;
	RefPtr<BufferResource>	m_constantBuffer;
	RefPtr<BindingStateImpl>    m_bindingState;
    int m_numAddedConstantBuffers;                      ///< Constant buffers can be added to the binding directly. Will be added at the end.
};

class ShaderObjectRenderTestApp : public RenderTestApp
{
public:
    virtual void applyBinding(PipelineType pipelineType) SLANG_OVERRIDE;
    virtual void setProjectionMatrix() SLANG_OVERRIDE;
    virtual Result initialize(
        SlangSession* session,
        Renderer* renderer,
        const Options& options,
        const ShaderCompilerUtil::Input& input) SLANG_OVERRIDE;
    virtual Result writeBindingOutput(BindRoot* bindRoot, const char* fileName) override;

protected:
    RefPtr<ProgramVars> m_programVars;
    ShaderOutputPlan m_outputPlan;
};

struct ShaderOffset
{
    SlangInt uniformOffset = 0;
    SlangInt bindingRangeIndex = 0;
    SlangInt bindingArrayIndex = 0;
};

class ShaderObjectLayout : public RefObject
{
public:
    struct BindingRangeInfo
    {
        slang::BindingType bindingType;
        Index   count;
        Index   baseIndex;
        Index   descriptorSetIndex;
        Index   rangeIndexInDescriptorSet;
//        Index   subObjectRangeIndex = -1;
    };

    struct SubObjectRangeInfo
    {
        RefPtr<ShaderObjectLayout>  layout;
//        Index                       baseIndex;
//        Index                       count;
        Index                       bindingRangeIndex;
    };

    struct DescriptorSetInfo : public RefObject
    {
        RefPtr<DescriptorSetLayout> layout;
        Slang::Int                  space = -1;
    };

    struct Builder
    {
    public:
        Builder(Renderer* renderer)
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
            if(m_mapSpaceToDescriptorSetIndex.TryGetValue(space, index))
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
            switch(slangBindingType)
            {
            default: return DescriptorSlotType::Unknown;

        #define CASE(FROM, TO) \
            case slang::BindingType::FROM: return DescriptorSlotType::TO

            CASE(Sampler,                           Sampler);
            CASE(CombinedTextureSampler,            CombinedImageSampler);
            CASE(Texture,                           SampledImage);
            CASE(MutableTexture,                    StorageImage);
            CASE(TypedBuffer,                       UniformTexelBuffer);
            CASE(MutableTypedBuffer,                StorageTexelBuffer);
            CASE(RawBuffer,                         UniformBuffer);
            CASE(MutableRawBuffer,                  StorageBuffer);
            CASE(InputRenderTarget,                 InputAttachment);
            CASE(InlineUniformData,                 InlineUniformBlock);
            CASE(RayTracingAccelerationStructure,   RayTracingAccelerationStructure);
            CASE(ConstantBuffer,                    UniformBuffer);
            CASE(PushConstant,                      RootConstant);

        #undef CASE
            }
        }

        slang::TypeLayoutReflection* unwrapParameterGroups(slang::TypeLayoutReflection* typeLayout)
        {
            for(;;)
            {
                if(!typeLayout->getType())
                {
                    if(auto elementTypeLayout = typeLayout->getElementTypeLayout())
                        typeLayout = elementTypeLayout;
                }

                switch(typeLayout->getKind())
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

        void _addDescriptorSets(slang::TypeLayoutReflection* typeLayout, slang::VariableLayoutReflection* varLayout = nullptr)
        {
            SlangInt descriptorSetCount = typeLayout->getDescriptorSetCount();
            for(SlangInt s = 0; s < descriptorSetCount; ++s)
            {
                auto descriptorSetIndex = findOrAddDescriptorSet(typeLayout->getDescriptorSetSpaceOffset(s));
                auto descriptorSetInfo = m_descriptorSetBuildInfos[descriptorSetIndex];

                SlangInt descriptorRangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(s);
                for(SlangInt r = 0; r < descriptorRangeCount; ++r)
                {
                    auto slangBindingType = typeLayout->getDescriptorSetDescriptorRangeType(s, r);
                    auto gfxDescriptorType = _mapDescriptorType(slangBindingType);

                    DescriptorSetLayout::SlotRangeDesc descriptorRangeDesc;
                    descriptorRangeDesc.binding = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(s, r);
                    descriptorRangeDesc.count = typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(s, r);
                    descriptorRangeDesc.type = gfxDescriptorType;

                    if(varLayout)
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
            for(SlangInt r = 0; r < bindingRangeCount; ++r)
            {
                slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                SlangInt count = typeLayout->getBindingRangeBindingCount(r);
                slang::TypeLayoutReflection* slangLeafTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(r);

                SlangInt descriptorSetIndex = typeLayout->getBindingRangeDescriptorSetIndex(r);
                SlangInt rangeIndexInDescriptorSet = typeLayout->getBindingRangeFirstDescriptorRangeIndex(r);

                Index baseIndex = 0;
                switch(slangBindingType)
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
            for(SlangInt r = 0; r < subObjectRangeCount; ++r)
            {
                SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
                auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
                slang::TypeLayoutReflection* slangLeafTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

                // A sub-object range can either represent a sub-object of a known
                // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
                // (in which case we can pre-compute a layout to use, based on
                // the type `Foo`) *or* it can represent a sub-object of some
                // existential type (e.g., `IBar`) in which case we cannot
                // know the appropraite type/layout of sub-object to allocate.
                //
                RefPtr<ShaderObjectLayout> subObjectLayout;
                if(slangBindingType != slang::BindingType::ExistentialValue)
                {
                    ShaderObjectLayout::createForElementType(m_renderer, slangLeafTypeLayout->getElementTypeLayout(), subObjectLayout.writeRef());
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

        SlangResult build(ShaderObjectLayout** outLayout)
        {
            RefPtr<ShaderObjectLayout> layout = new ShaderObjectLayout();
            SLANG_RETURN_ON_FAIL(layout->_init(this));

            *outLayout = layout.detach();
            return SLANG_OK;
        }

        Renderer* m_renderer = nullptr;
        slang::TypeLayoutReflection* m_elementTypeLayout = nullptr;
    };

    static Result createForElementType(Renderer* renderer, slang::TypeLayoutReflection* elementType, ShaderObjectLayout** outLayout)
    {
        Builder builder(renderer);
        builder.setElementTypeLayout(elementType);
        return builder.build(outLayout);
    }

    List<RefPtr<DescriptorSetInfo>> const& getDescriptorSets() { return m_descriptorSets; }

    List<BindingRangeInfo> const& getBindingRanges() { return m_bindingRanges; }

    Index getBindingRangeCount() { return m_bindingRanges.getCount(); }

    BindingRangeInfo const& getBindingRange(Index index) { return m_bindingRanges[index]; }

    slang::TypeLayoutReflection* getElementTypeLayout()
    {
        return m_elementTypeLayout;
    }

    Index getResourceViewCount() { return m_resourceViewCount; }
    Index getSamplerCount() { return m_samplerCount; }
    Index getCombinedTextureSamplerCount() { return m_combinedTextureSamplerCount; }
    Index getSubObjectCount() { return m_subObjectCount; }

    SubObjectRangeInfo const& getSubObjectRange(Index index) { return m_subObjectRanges[index]; }
    List<SubObjectRangeInfo> const& getSubObjectRanges() { return m_subObjectRanges; }

    Renderer* getRenderer()
    {
        return m_renderer;
    }

protected:
    Result _init(Builder const* builder)
    {
        auto renderer = builder->m_renderer;
        m_renderer = renderer;

        m_elementTypeLayout = builder->m_elementTypeLayout;
        m_bindingRanges = builder->m_bindingRanges;

        for(auto descriptorSetBuildInfo : builder->m_descriptorSetBuildInfos)
        {
            auto& slotRangeDescs = descriptorSetBuildInfo->slotRangeDescs;
            DescriptorSetLayout::Desc desc;
            desc.slotRangeCount = slotRangeDescs.getCount();
            desc.slotRanges = slotRangeDescs.getBuffer();

            RefPtr<DescriptorSetLayout> descriptorSetLayout;
            SLANG_RETURN_ON_FAIL(m_renderer->createDescriptorSetLayout(desc, descriptorSetLayout.writeRef()));

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

    Renderer*                       m_renderer;
    List<RefPtr<DescriptorSetInfo>> m_descriptorSets;
    List<BindingRangeInfo>          m_bindingRanges;
    slang::TypeLayoutReflection*    m_elementTypeLayout;
    Index                           m_resourceViewCount = 0;
    Index                           m_samplerCount = 0;
    Index                           m_combinedTextureSamplerCount = 0;
    Index                           m_subObjectCount = 0;
    List<SubObjectRangeInfo>        m_subObjectRanges;
};

class EntryPointLayout : public ShaderObjectLayout
{
    typedef ShaderObjectLayout Super;
public:

    struct VaryingInputInfo
    {
    };

    struct VaryingOutputInfo
    {
    };

    struct Builder : Super::Builder
    {
        Builder(Renderer* renderer)
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
            for(SlangInt r = 0; r < bindingRangeCount; ++r)
            {
                slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                SlangInt count = typeLayout->getBindingRangeBindingCount(r);

                switch(slangBindingType)
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

class ProgramLayout : public ShaderObjectLayout
{
    typedef ShaderObjectLayout Super;
public:

    struct EntryPointInfo
    {
        RefPtr<EntryPointLayout>    layout;
        Index                       rangeOffset;
    };


    struct Builder : Super::Builder
    {
        Builder(Renderer* renderer)
            : Super::Builder(renderer)
        {}

        Result build(ProgramLayout** outLayout)
        {
            RefPtr<ProgramLayout> layout = new ProgramLayout();
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

            if(m_descriptorSetBuildInfos.getCount())
            {
                info.rangeOffset = m_descriptorSetBuildInfos[0]->slotRangeDescs.getCount();
            }

            auto slangEntryPointLayout = entryPointLayout->getSlangLayout();
            _addDescriptorSets(slangEntryPointLayout->getTypeLayout(), slangEntryPointLayout->getVarLayout());

            m_entryPoints.add(info);
        }

        List<EntryPointInfo> m_entryPoints;
    };

    Slang::Int getRenderTargetCount()
    {
        return m_renderTargetCount;
    }

    PipelineLayout* getPipelineLayout() { return m_pipelineLayout; }

    Index findEntryPointIndex(gfx::StageType stage)
    {
        auto entryPointCount = m_entryPoints.getCount();
        for(Index i = 0; i < entryPointCount; ++i)
        {
            auto entryPoint = m_entryPoints[i];
            if(entryPoint.layout->getStage() == stage)
                return i;
        }
        return -1;
    }

    EntryPointInfo const& getEntryPoint(Index index)
    {
        return m_entryPoints[index];
    }

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
        if(fragmentEntryPointIndex != -1)
        {
            auto fragmentEntryPoint = getEntryPoint(fragmentEntryPointIndex);
            m_renderTargetCount = fragmentEntryPoint.layout->getVaryingOutputs().getCount();
        }

        PipelineLayout::Desc pipelineLayoutDesc;
        pipelineLayoutDesc.renderTargetCount = m_renderTargetCount;
        pipelineLayoutDesc.descriptorSetCount = pipelineDescriptorSets.getCount();
        pipelineLayoutDesc.descriptorSets = pipelineDescriptorSets.getBuffer();

        SLANG_RETURN_ON_FAIL(renderer->createPipelineLayout(pipelineLayoutDesc, m_pipelineLayout.writeRef()));

        return SLANG_OK;
    }

    static void _addDescriptorSetsRec(ShaderObjectLayout* layout, List<PipelineLayout::DescriptorSetDesc>& ioPipelineDescriptorSets)
    {
        for(auto descriptorSetInfo : layout->getDescriptorSets())
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

class ShaderObject : public RefObject
{
public:
    static Result create(Renderer* renderer, ShaderObjectLayout* layout, ShaderObject** outShaderObject)
    {
        RefPtr<ShaderObject> object = new ShaderObject();
        SLANG_RETURN_ON_FAIL(object->init(renderer, layout));

        *outShaderObject = object.detach();
        return SLANG_OK;
    }

    Renderer* getRenderer()
    {
        return m_layout->getRenderer();
    }

    ShaderObjectLayout* getLayout()
    {
        return m_layout;
    }

    slang::TypeLayoutReflection* getElementTypeLayout()
    {
        return m_layout->getElementTypeLayout();
    }

    SlangResult setData(ShaderOffset const& offset, void const* data, size_t size)
    {
        Renderer* renderer = getRenderer();

        char* dest = (char*) renderer->map(m_buffer, MapFlavor::HostWrite);
        memcpy(dest + offset.uniformOffset, data, size);
        renderer->unmap(m_buffer);

        return SLANG_OK;
    }

    SlangResult setObject(ShaderOffset const& offset, ShaderObject* object)
    {
        if(offset.bindingRangeIndex < 0) return SLANG_E_INVALID_ARG;
        if(offset.bindingRangeIndex >= m_layout->getBindingRangeCount()) return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        // TODO: Is this reasonable to store the base index directly in the binding range?
        m_objects[bindingRange.baseIndex + offset.bindingArrayIndex] = object;

//        auto& subObjectRange = m_layout->getSubObjectRange(bindingRange.subObjectRangeIndex);
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

    SlangResult getObject(ShaderOffset const& offset, ShaderObject** outObject)
    {
        SLANG_ASSERT(outObject);
        if(offset.bindingRangeIndex < 0) return SLANG_E_INVALID_ARG;
        if(offset.bindingRangeIndex >= m_layout->getBindingRangeCount()) return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        *outObject = m_objects[bindingRange.baseIndex + offset.bindingArrayIndex];

//        auto& subObjectRange = m_layout->getSubObjectRange(bindingRange.subObjectRangeIndex);
//        *outObject = m_objects[subObjectRange.baseIndex + offset.bindingArrayIndex];

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
        if(offset.bindingRangeIndex < 0) return SLANG_E_INVALID_ARG;
        if(offset.bindingRangeIndex >= m_layout->getBindingRangeCount()) return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        m_resourceViews[bindingRange.baseIndex + offset.bindingArrayIndex] = resourceView;
        return SLANG_OK;
    }

    SlangResult setSampler(ShaderOffset const& offset, SamplerState* sampler)
    {
        if(offset.bindingRangeIndex < 0) return SLANG_E_INVALID_ARG;
        if(offset.bindingRangeIndex >= m_layout->getBindingRangeCount()) return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        m_samplers[bindingRange.baseIndex + offset.bindingArrayIndex] = sampler;
        return SLANG_OK;
    }

    SlangResult setCombinedTextureSampler(ShaderOffset const& offset, ResourceView* textureView, SamplerState* sampler)
    {
        if(offset.bindingRangeIndex < 0) return SLANG_E_INVALID_ARG;
        if(offset.bindingRangeIndex >= m_layout->getBindingRangeCount()) return SLANG_E_INVALID_ARG;
        auto& bindingRange = m_layout->getBindingRange(offset.bindingRangeIndex);

        auto& slot = m_combinedTextureSamplers[bindingRange.baseIndex + offset.bindingArrayIndex];
        slot.textureView = textureView;
        slot.sampler = sampler;
        return SLANG_OK;
    }

protected:
    friend class ProgramVars;

    Result init(Renderer* renderer, ShaderObjectLayout* layout)
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
        if(uniformSize)
        {
            BufferResource::Desc bufferDesc;
            bufferDesc.init(uniformSize);
            bufferDesc.cpuAccessFlags |= Resource::AccessFlag::Write;
            SLANG_RETURN_ON_FAIL(renderer->createBufferResource(Resource::Usage::ConstantBuffer, bufferDesc, nullptr, m_buffer.writeRef()));
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

        for(auto subObjectRangeInfo : layout->getSubObjectRanges())
        {
            RefPtr<ShaderObjectLayout> subObjectLayout = subObjectRangeInfo.layout;

            // In the case where the sub-object range represents an
            // existential-type leaf field (e.g., an `IBar`), we
            // cannot pre-allocate the objet(s) to go into that
            // range, since we can't possibly know what to allocate
            // at this point.
            //
            if(!subObjectLayout)
                continue;
            //
            // Otherwise, we will allocate a sub-object to fill
            // in each entry in this range, based on the layout
            // information we already have.

            auto& bindingRangeInfo = layout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);
            for(Index i = 0; i < bindingRangeInfo.count; ++i)
            {
                RefPtr<ShaderObject> subObject;
                SLANG_RETURN_ON_FAIL(ShaderObject::create(renderer, subObjectLayout, subObject.writeRef()));
                m_objects[bindingRangeInfo.baseIndex + i] = subObject;
            }
        }

        return SLANG_OK;
    }

    Result apply(Renderer* renderer, PipelineType pipelineType, PipelineLayout* pipelineLayout, Index& ioRootIndex)
    {
        ShaderObjectLayout* layout = m_layout;

        // Create the descritpor sets required by the layout...
        //
        List<RefPtr<DescriptorSet>> descriptorSets;
        for(auto descriptorSetInfo : layout->getDescriptorSets())
        {
            RefPtr<DescriptorSet> descriptorSet;
            SLANG_RETURN_ON_FAIL(renderer->createDescriptorSet(descriptorSetInfo->layout, descriptorSet.writeRef()));
            descriptorSets.add(descriptorSet);
        }

        SLANG_RETURN_ON_FAIL(_bindIntoDescriptorSets(descriptorSets.getBuffer()));

        for(auto descriptorSet : descriptorSets)
        {
            renderer->setDescriptorSet(pipelineType, pipelineLayout, ioRootIndex++, descriptorSet);
        }

        return SLANG_OK;
    }

    Result _bindIntoDescriptorSet(DescriptorSet* descriptorSet, Index baseRangeIndex, Index subObjectRangeArrayIndex)
    {
        ShaderObjectLayout* layout = m_layout;

        if(m_buffer)
        {
            descriptorSet->setConstantBuffer(baseRangeIndex, subObjectRangeArrayIndex, m_buffer);
            baseRangeIndex++;
        }

        for(auto bindingRangeInfo : layout->getBindingRanges())
        {
            switch(bindingRangeInfo.bindingType)
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

            switch(bindingRangeInfo.bindingType)
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
                for(Index i = 0; i < count; ++i)
                {
                    auto& slot = m_combinedTextureSamplers[baseIndex + i];
                    descriptorSet->setCombinedTextureSampler(descriptorRangeIndex, descriptorArrayBaseIndex + i, slot.textureView, slot.sampler);
                }
                break;

            case slang::BindingType::Sampler:
                for(Index i = 0; i < count; ++i)
                {
                    descriptorSet->setSampler(descriptorRangeIndex, descriptorArrayBaseIndex + i, m_samplers[baseIndex + i]);
                }
                break;

            default:
                for(Index i = 0; i < count; ++i)
                {
                    descriptorSet->setResource(descriptorRangeIndex, descriptorArrayBaseIndex + i, m_resourceViews[baseIndex + i]);
                }
                break;
            }
        }

        return SLANG_OK;
    }

    public:
    virtual Result _bindIntoDescriptorSets(RefPtr<DescriptorSet>* descriptorSets)
    {
        ShaderObjectLayout* layout = m_layout;

        if(m_buffer)
        {
            // TODO: look up binding infor for default constant buffer...
            descriptorSets[0]->setConstantBuffer(0, 0, m_buffer);
        }

        // Fill in the descriptor sets based on binding ranges
        //
        for(auto bindingRangeInfo : layout->getBindingRanges())
        {
            DescriptorSet* descriptorSet = descriptorSets[bindingRangeInfo.descriptorSetIndex];
            auto rangeIndex = bindingRangeInfo.rangeIndexInDescriptorSet;
            auto baseIndex = bindingRangeInfo.baseIndex;
            auto count = bindingRangeInfo.count;
            switch(bindingRangeInfo.bindingType)
            {
            case slang::BindingType::ConstantBuffer:
            case slang::BindingType::ParameterBlock:
                for(Index i = 0; i < count; ++i)
                {
                    ShaderObject* subObject = m_objects[baseIndex + i];

                    subObject->_bindIntoDescriptorSet(descriptorSet, rangeIndex, i);
                }
                break;

            case slang::BindingType::CombinedTextureSampler:
                for(Index i = 0; i < count; ++i)
                {
                    auto& slot = m_combinedTextureSamplers[baseIndex + i];
                    descriptorSet->setCombinedTextureSampler(rangeIndex, i, slot.textureView, slot.sampler);
                }
                break;

            case slang::BindingType::Sampler:
                for(Index i = 0; i < count; ++i)
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
                for(Index i = 0; i < count; ++i)
                {
                    ShaderObject* subObject = m_objects[baseIndex + i];

                    subObject->_bindIntoDescriptorSet(descriptorSet, rangeIndex, i);
                }
                break;

            default:
                for(Index i = 0; i < count; ++i)
                {
                    descriptorSet->setResource(rangeIndex, i, m_resourceViews[baseIndex + i]);
                }
                break;
            }
        }
        return SLANG_OK;
    }


    RefPtr<ShaderObjectLayout> m_layout = nullptr;
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
    List<RefPtr<ShaderObject>> m_objects;
};

class EntryPointVars : public ShaderObject
{
    typedef ShaderObject Super;

public:
    static Result create(Renderer* renderer, EntryPointLayout* layout, EntryPointVars** outShaderObject)
    {
        RefPtr<EntryPointVars> object = new EntryPointVars();
        SLANG_RETURN_ON_FAIL(object->init(renderer, layout));

        *outShaderObject = object.detach();
        return SLANG_OK;
    }

    EntryPointLayout* getLayout()
    {
        return static_cast<EntryPointLayout*>(m_layout.Ptr());
    }

protected:
    Result init(Renderer* renderer, EntryPointLayout* layout)
    {
        SLANG_RETURN_ON_FAIL(Super::init(renderer, layout));
        return SLANG_OK;
    }
};

class ProgramVars : public ShaderObject
{
    typedef ShaderObject Super;

public:
    static Result create(Renderer* renderer, ProgramLayout* layout, ProgramVars** outShaderObject)
    {
        RefPtr<ProgramVars> object = new ProgramVars();
        SLANG_RETURN_ON_FAIL(object->init(renderer, layout));

        *outShaderObject = object.detach();
        return SLANG_OK;
    }

    ProgramLayout* getLayout()
    {
        return static_cast<ProgramLayout*>(m_layout.Ptr());
    }

    void apply(Renderer* renderer, PipelineType pipelineType)
    {
        auto pipelineLayout = getLayout()->getPipelineLayout();

        Index rootIndex = 0;
        ShaderObject::apply(renderer, pipelineType, pipelineLayout, rootIndex);

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

protected:

    virtual Result _bindIntoDescriptorSets(RefPtr<DescriptorSet>* descriptorSets)
    {
        SLANG_RETURN_ON_FAIL(Super::_bindIntoDescriptorSets(descriptorSets));

        auto entryPointCount = m_entryPoints.getCount();
        for( Index i = 0; i < entryPointCount; ++i )
        {
            auto entryPoint = m_entryPoints[i];
            auto& entryPointInfo = getLayout()->getEntryPoint(i);

            SLANG_RETURN_ON_FAIL(entryPoint->_bindIntoDescriptorSet(descriptorSets[0], entryPointInfo.rangeOffset, 0));
        }

        return SLANG_OK;
    }


    Result init(Renderer* renderer, ProgramLayout* layout)
    {
        SLANG_RETURN_ON_FAIL(Super::init(renderer, layout));

        for(auto entryPointInfo : layout->getEntryPoints())
        {
            RefPtr<EntryPointVars> entryPoint;
            SLANG_RETURN_ON_FAIL(EntryPointVars::create(renderer, entryPointInfo.layout, entryPoint.writeRef()));
            m_entryPoints.add(entryPoint);
        }

        return SLANG_OK;
    }

    List<RefPtr<EntryPointVars>> m_entryPoints;
};

    /// Represents a "pointer" to the storage for a shader parameter of a (dynamically) known type.
    ///
    /// A `ShaderCursor` serves as a pointer-like type for things stored inside a `ShaderObject`.
    ///
    /// A cursor that points to the entire content of a shader object can be formed as `ShaderCursor(someObject)`.
    /// A cursor pointing to a structure field or array element can be formed from another cursor
    /// using `getField` or `getElement` respectively.
    ///
    /// Given a cursor pointing to a value of some "primitive" type, we can set or get the value
    /// using operations like `setResource`, `getResource`, etc.
    ///
    /// Because type information for shader parameters is being reflected dynamically, all type
    /// checking for shader cursors occurs at runtime, and errors may occur when attempting to
    /// set a parameter using a value of an inappropriate type. As much as possible, `ShaderCursor`
    /// attempts to protect against these cases and return an error `Result` or an invalid
    /// cursor, rather than allowing operations to proceed with incorrect types.
    ///
struct ShaderCursor
{
    ShaderObject*                   m_baseObject = nullptr;
    slang::TypeLayoutReflection*    m_typeLayout = nullptr;
    ShaderOffset                    m_offset;

        /// Get the type (layout) of the value being pointed at by the cursor
    slang::TypeLayoutReflection* getTypeLayout() const
    {
        return m_typeLayout;
    }

        /// Is this cursor valid (that is, does it seem to point to an actual location)?
        ///
        /// This check is equivalent to checking whether a pointer is null, so it is
        /// a very weak sense of "valid." In particular, it is possible to form a
        /// `ShaderCursor` for which `isValid()` is true, but attempting to get or
        /// set the value would be an error (like dereferencing a garbage pointer).
        ///
    bool isValid() const
    {
        return m_baseObject != nullptr;
    }

    Result getDereferenced(ShaderCursor& outCursor) const
    {
        switch(m_typeLayout->getKind())
        {
        default:
            return SLANG_E_INVALID_ARG;

        case slang::TypeReflection::Kind::ConstantBuffer:
        case slang::TypeReflection::Kind::ParameterBlock:
            {
                ShaderObject* subObject = m_baseObject->getObject(m_offset);
                outCursor = ShaderCursor(subObject);
                return SLANG_OK;
            }
        
        }
    }

    ShaderCursor getDereferenced()
    {
        ShaderCursor result;
        getDereferenced(result);
        return result;
    }

        /// Form a cursor pointing to the field with the given `name` within the value this cursor points at.
        ///
        /// If the operation succeeds, then the field cursor is written to `outCursor`.
    Result getField(UnownedStringSlice const& name, ShaderCursor& outCursor)
    {
        // If this cursor is invalid, then can't possible fetch a field.
        //
        if(!isValid()) return SLANG_E_INVALID_ARG;

        // If the cursor is valid, we want to consider the type of data
        // it is referencing.
        //
        switch(m_typeLayout->getKind())
        {
            // The easy/expected case is when the value has a structure type.
            //
        case slang::TypeReflection::Kind::Struct:
            {
                // We start by looking up the index of a field matching `name`.
                //
                // If there is no such field, we have an error.
                //
                SlangInt fieldIndex = m_typeLayout->findFieldIndexByName(name.begin(), name.end());
                if(fieldIndex == -1)
                    return SLANG_E_INVALID_ARG;

                // Once we know the index of the field being referenced,
                // we create a cursor to point at the field, based on
                // the offset information already in this cursor, plus
                // offsets derived from the field's layout.
                //
                slang::VariableLayoutReflection* fieldLayout = m_typeLayout->getFieldByIndex((unsigned int) fieldIndex);
                ShaderCursor fieldCursor;

                // The field cursorwill point into the same parent object.
                //
                fieldCursor.m_baseObject = m_baseObject;

                // The type being pointed to is the tyep of the field.
                //
                fieldCursor.m_typeLayout = fieldLayout->getTypeLayout();

                // The byte offset is the current offset plus the relative offset of the field.
                // The offset in binding ranges is computed similarly.
                //
                fieldCursor.m_offset.uniformOffset = m_offset.uniformOffset + fieldLayout->getOffset();
                fieldCursor.m_offset.bindingRangeIndex = m_offset.bindingRangeIndex +  m_typeLayout->getFieldBindingRangeOffset(fieldIndex);

                // The index of the field within any binding ranges will be the same
                // as the index computed for the parent structure.
                //
                // Note: this case would arise for an array of structures with texture-type
                // fields. Suppose we have:
                //
                //      struct S { Texture2D t; Texture2D u; }
                //      S g[4];
                //
                // In this scenario, `g` holds two binding ranges:
                //
                // * Range #0 comprises 4 textures, representing `g[...].t`
                // * Range #1 comprises 4 textures, representing `g[...].u`
                //
                // A cursor for `g[2]` would have a `bindingRangeIndex` of zero but
                // a `bindingArrayIndex` of 2, iindicating that we could end up
                // referencing either range, but no matter what we know the index
                // is 2. Thus when we form a cursor for `g[2].u` we want to
                // apply the binding range offset to get a `bindingRangeIndex` of
                // 1, while the `bindingArrayIndex` is unmodified.
                //
                // The result is that `g[2].u` is stored in range #1 at array index 2.
                //
                fieldCursor.m_offset.bindingArrayIndex = m_offset.bindingArrayIndex;

                outCursor = fieldCursor;
                return SLANG_OK;
            }
            break;

        // In some cases the user might be trying to acess a field by name
        // from a cursor that references a constant buffer or parameter block,
        // and in these cases we want the access to Just Work.
        //
        case slang::TypeReflection::Kind::ConstantBuffer:
        case slang::TypeReflection::Kind::ParameterBlock:
            {
                // We basically need to "dereference" the current cursor
                // to go from a pointer to a constant buffer to a pointer
                // to the *contents* of the constant buffer.
                //
                ShaderCursor d = getDereferenced();
                return d.getField(name, outCursor);
            }
            break;
        }

        return SLANG_E_INVALID_ARG;
    }

    ShaderCursor getField(UnownedStringSlice const& name)
    {
        ShaderCursor cursor;
        getField(name, cursor);
        return cursor;
    }

    ShaderCursor getField(String const& name)
    {
        return getField(name.getUnownedSlice());
    }

    ShaderCursor getElement(Index index)
    {
        // TODO: need to auto-dereference various buffer types...

        if(m_typeLayout->getKind() == slang::TypeReflection::Kind::Array)
        {
            ShaderCursor elementCursor;
            elementCursor.m_baseObject = m_baseObject;
            elementCursor.m_typeLayout = m_typeLayout->getElementTypeLayout();
            elementCursor.m_offset.uniformOffset = m_offset.uniformOffset + index * m_typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
            elementCursor.m_offset.bindingRangeIndex = m_offset.bindingRangeIndex;
            elementCursor.m_offset.bindingArrayIndex = m_offset.bindingArrayIndex*m_typeLayout->getElementCount() + index;
            return elementCursor;
        }

        return ShaderCursor();
    }

    static int _peek(UnownedStringSlice const& slice)
    {
        const char* b = slice.begin();
        const char* e = slice.end();
        if(b == e) return -1;
        return *b;
    }

    static int _get(UnownedStringSlice& slice)
    {
        const char* b = slice.begin();
        const char* e = slice.end();
        if(b == e) return -1;
        auto result = *b++;
        slice = UnownedStringSlice(b, e);
        return result;
    }

    static Result followPath(UnownedStringSlice const& path, ShaderCursor& ioCursor)
    {
        ShaderCursor cursor = ioCursor;

        enum
        {
            ALLOW_NAME = 0x1,
            ALLOW_SUBSCRIPT = 0x2,
            ALLOW_DOT = 0x4,
        };
        int state = ALLOW_NAME | ALLOW_SUBSCRIPT;

        UnownedStringSlice rest = path;
        for(;;)
        {
            int c = _peek(rest);

            if(c == -1)
                break;
            else if( c == '.' )
            {
                if(!(state & ALLOW_DOT))
                    return SLANG_E_INVALID_ARG;

                _get(rest);
                state = ALLOW_NAME;
                continue;
            }
            else if( c == '[' )
            {
                if(!(state & ALLOW_SUBSCRIPT))
                    return SLANG_E_INVALID_ARG;

                _get(rest);
                Index index = 0;
                while(_peek(rest) != ']')
                {
                    int d = _get(rest);
                    if(d >= '0' && d <= '9')
                    {
                        index = index*10 + (d - '0');
                    }
                    else
                    {
                        return SLANG_E_INVALID_ARG;
                    }
                }

                if(_peek(rest) != ']')
                    return SLANG_E_INVALID_ARG;
                _get(rest);

                cursor = cursor.getElement(index);
                state = ALLOW_DOT | ALLOW_SUBSCRIPT;
                continue;
            }
            else
            {
                const char* nameBegin = rest.begin();
                for(;;)
                {
                    switch(_peek(rest))
                    {
                    default:
                        _get(rest);
                        continue;

                    case -1:
                    case '.':
                    case '[':
                        break;
                    }
                    break;
                }
                char const* nameEnd = rest.begin();
                UnownedStringSlice name(nameBegin, nameEnd);
                cursor = cursor.getField(name);
                state = ALLOW_DOT | ALLOW_SUBSCRIPT;
                continue;
            }
        }

        ioCursor = cursor;
        return SLANG_OK;
    }

    static Result followPath(String const& path, ShaderCursor& ioCursor)
    {
        return followPath(path.getUnownedSlice(), ioCursor);
    }

    ShaderCursor getPath(UnownedStringSlice const& path)
    {
        ShaderCursor result(*this);
        followPath(path, result);
        return result;
    }


    ShaderCursor getPath(String const& path)
    {
        ShaderCursor result(*this);
        followPath(path, result);
        return result;
    }

    ShaderCursor()
    {}

    ShaderCursor(ShaderObject* object)
        : m_baseObject(object)
        , m_typeLayout(object->getElementTypeLayout())
    {}

    SlangResult setData(void const* data, size_t size)
    {
        return m_baseObject->setData(m_offset, data, size);
    }

    SlangResult setObject(ShaderObject* object)
    {
        return m_baseObject->setObject(m_offset, object);
    }

    SlangResult setResource(ResourceView* resourceView)
    {
        return m_baseObject->setResource(m_offset, resourceView);
    }

    SlangResult setSampler(SamplerState* sampler)
    {
        return m_baseObject->setSampler(m_offset, sampler);
    }

    SlangResult setCombinedTextureSampler(ResourceView* textureView, SamplerState* sampler)
    {
        return m_baseObject->setCombinedTextureSampler(m_offset, textureView, sampler);
    }
};

SlangResult _assignVarsFromLayout(
    Renderer*                   renderer,
    ProgramVars*                shaderObject,
    ShaderInputLayout const&    layout,
    ShaderOutputPlan&           ioOutputPlan,
    slang::ProgramLayout*       slangReflection)
{
    ShaderCursor rootCursor = ShaderCursor(shaderObject);

    const int textureBindFlags = Resource::BindFlag::NonPixelShaderResource | Resource::BindFlag::PixelShaderResource;

    Index entryCount = layout.entries.getCount();
    for(Index entryIndex = 0; entryIndex < entryCount; ++entryIndex)
    {
        auto& entry = layout.entries[entryIndex];
        if(entry.name.getLength() == 0)
        {
            StdWriters::getError().print("error: entries in `ShaderInputLayout` must include a name\n");
            return SLANG_E_INVALID_ARG;
        }

        auto entryCursor = rootCursor.getPath(entry.name);

        if(!entryCursor.isValid())
        {
            for(auto entryPoint : shaderObject->getEntryPoints())
            {
                entryCursor = ShaderCursor(entryPoint).getPath(entry.name);
                if(entryCursor.isValid())
                    break;
            }
        }


        if(!entryCursor.isValid())
        {
            StdWriters::getError().print("error: could not find shader parameter matching '%s'\n", entry.name.begin());
            return SLANG_E_INVALID_ARG;
        }

        RefPtr<Resource> resource;
        switch(entry.type)
        {
        case ShaderInputType::Uniform:
            {
                const size_t bufferSize = entry.bufferData.getCount() * sizeof(uint32_t);

                ShaderCursor dataCursor = entryCursor;
                switch(dataCursor.getTypeLayout()->getKind())
                {
                case slang::TypeReflection::Kind::ConstantBuffer:
                case slang::TypeReflection::Kind::ParameterBlock:
                    dataCursor = dataCursor.getDereferenced();
                    break;

                default:
                    break;

                }

                dataCursor.setData(entry.bufferData.getBuffer(), bufferSize);
            }
            break;

        case ShaderInputType::Buffer:
            {
                const InputBufferDesc& srcBuffer = entry.bufferDesc;
                const size_t bufferSize = entry.bufferData.getCount() * sizeof(uint32_t);

                switch(srcBuffer.type)
                {
                case InputBufferType::ConstantBuffer:
                    {
                        // A `cbuffer` input line actually represents the data we
                        // want to write *into* the buffer, and shouldn't
                        // allocate a buffer itself.
                        //
                        entryCursor.getDereferenced().setData(entry.bufferData.getBuffer(), bufferSize);
                    }
                    break;

                case InputBufferType::RootConstantBuffer:
                    {
                        // A `root_constants` input line actually represents the data we
                        // want to write *into* the buffer, and shouldn't
                        // allocate a buffer itself.
                        //
                        // Note: we are not doing `.getDereferenced()` here because the
                        // `root_constant` line should be referring to a parameter value
                        // inside the root-constant range, and not the range/buffer itself.
                        //
                        entryCursor.setData(entry.bufferData.getBuffer(), bufferSize);
                    }
                    break;

                default:
                    {

                        RefPtr<BufferResource> bufferResource;
                        SLANG_RETURN_ON_FAIL(ShaderRendererUtil::createBufferResource(entry.bufferDesc, entry.isOutput, bufferSize, entry.bufferData.getBuffer(), renderer, bufferResource));
                        resource = bufferResource;

                        ResourceView::Desc viewDesc;
                        viewDesc.type = ResourceView::Type::UnorderedAccess;
                        viewDesc.format = srcBuffer.format;
                        auto bufferView = renderer->createBufferView(
                            bufferResource,
                            viewDesc);
                        entryCursor.setResource(bufferView);
                    }
                    break;
                }

#if 0
                switch(srcBuffer.type)
                {
                case InputBufferType::ConstantBuffer:
                    descriptorSet->setConstantBuffer(rangeIndex, 0, bufferResource);
                    break;

                case InputBufferType::StorageBuffer:
                    {
                        ResourceView::Desc viewDesc;
                        viewDesc.type = ResourceView::Type::UnorderedAccess;
                        viewDesc.format = srcBuffer.format;
                        auto bufferView = renderer->createBufferView(
                            bufferResource,
                            viewDesc);
                        descriptorSet->setResource(rangeIndex, 0, bufferView);
                    }
                    break;
                }

                if(srcEntry.isOutput)
                {
                    BindingStateImpl::OutputBinding binding;
                    binding.entryIndex = i;
                    binding.resource = bufferResource;
                    outputBindings.add(binding);
                }
#endif
            }
            break;

        case ShaderInputType::CombinedTextureSampler:
            {
                RefPtr<TextureResource> texture;
                SLANG_RETURN_ON_FAIL(ShaderRendererUtil::generateTextureResource(entry.textureDesc, textureBindFlags, renderer, texture));
                resource = texture;

                auto sampler = _createSamplerState(renderer, entry.samplerDesc);

                ResourceView::Desc viewDesc;
                viewDesc.type = ResourceView::Type::ShaderResource;
                auto textureView = renderer->createTextureView(
                    texture,
                    viewDesc);

                entryCursor.setCombinedTextureSampler(textureView, sampler);

#if 0
                descriptorSet->setCombinedTextureSampler(rangeIndex, 0, textureView, sampler);

                if(srcEntry.isOutput)
                {
                    BindingStateImpl::OutputBinding binding;
                    binding.entryIndex = i;
                    binding.resource = texture;
                    outputBindings.add(binding);
                }
#endif
            }
            break;

        case ShaderInputType::Texture:
            {
                RefPtr<TextureResource> texture;
                SLANG_RETURN_ON_FAIL(ShaderRendererUtil::generateTextureResource(entry.textureDesc, textureBindFlags, renderer, texture));
                resource = texture;

                // TODO: support UAV textures...

                ResourceView::Desc viewDesc;
                viewDesc.type = ResourceView::Type::ShaderResource;
                auto textureView = renderer->createTextureView(
                    texture,
                    viewDesc);

                if (!textureView)
                {
                    return SLANG_FAIL;
                }

                entryCursor.setResource(textureView);

#if 0
                descriptorSet->setResource(rangeIndex, 0, textureView);

                if(srcEntry.isOutput)
                {
                    BindingStateImpl::OutputBinding binding;
                    binding.entryIndex = i;
                    binding.resource = texture;
                    outputBindings.add(binding);
                }
#endif
            }
            break;

        case ShaderInputType::Sampler:
            {
                auto sampler = _createSamplerState(renderer, entry.samplerDesc);

                entryCursor.setSampler(sampler);
#if 0
                descriptorSet->setSampler(rangeIndex, 0, sampler);
#endif
            }
            break;

        case ShaderInputType::Object:
            {
                auto typeName = entry.objectDesc.typeName;
                auto slangType = slangReflection->findTypeByName(typeName.getBuffer());
                auto slangTypeLayout = slangReflection->getTypeLayout(slangType);

                RefPtr<ShaderObjectLayout> shaderObjectLayout;
                SLANG_RETURN_ON_FAIL(ShaderObjectLayout::createForElementType(renderer, slangTypeLayout, shaderObjectLayout.writeRef()));

                RefPtr<ShaderObject> shaderObject;
                SLANG_RETURN_ON_FAIL(ShaderObject::create(renderer, shaderObjectLayout, shaderObject.writeRef()));

                entryCursor.setObject(shaderObject);
            }
            break;

        default:
            assert(!"Unhandled type");
            return SLANG_FAIL;
        }

        if(entry.isOutput)
        {
            ShaderOutputPlan::Item item;
            item.inputLayoutEntryIndex = entryIndex;
            item.resource = resource;
            ioOutputPlan.items.add(item);
        }

    }
    return SLANG_OK;
}

void LegacyRenderTestApp::applyBinding(PipelineType pipelineType)
{
    m_bindingState->apply(m_renderer.Ptr(), pipelineType);
}

void ShaderObjectRenderTestApp::applyBinding(PipelineType pipelineType)
{
    m_programVars->apply(m_renderer.Ptr(), pipelineType);
}

SlangResult LegacyRenderTestApp::initialize(
    SlangSession* session,
    Renderer* renderer,
    const Options& options,
    const ShaderCompilerUtil::Input& input)
{
    m_options = options;

    SLANG_RETURN_ON_FAIL(_initializeShaders(session, renderer, options.shaderType, input));

    m_numAddedConstantBuffers = 0;
    m_renderer = renderer;

    // TODO(tfoley): use each API's reflection interface to query the constant-buffer size needed
    m_constantBufferSize = 16 * sizeof(float);

    BufferResource::Desc constantBufferDesc;
    constantBufferDesc.init(m_constantBufferSize);
    constantBufferDesc.cpuAccessFlags = Resource::AccessFlag::Write;

    m_constantBuffer =
        renderer->createBufferResource(Resource::Usage::ConstantBuffer, constantBufferDesc);
    if (!m_constantBuffer)
        return SLANG_FAIL;

    //! Hack -> if doing a graphics test, add an extra binding for our dynamic constant buffer
    //
    // TODO: Should probably be more sophisticated than this - with 'dynamic' constant buffer/s
    // binding always being specified in the test file
    RefPtr<BufferResource> addedConstantBuffer;
    switch (m_options.shaderType)
    {
    default:
        break;

    case Options::ShaderProgramType::Graphics:
    case Options::ShaderProgramType::GraphicsCompute:
        addedConstantBuffer = m_constantBuffer;
        m_numAddedConstantBuffers++;
        break;
    }

    BindingStateImpl* bindingState = nullptr;
    SLANG_RETURN_ON_FAIL(ShaderRendererUtil::createBindingState(
        m_shaderInputLayout, m_renderer, addedConstantBuffer, &bindingState));
    m_bindingState = bindingState;

    // Do other initialization that doesn't depend on the source language.

    // Input Assembler (IA)

    const InputElementDesc inputElements[] = {
        {"A", 0, Format::RGB_Float32, offsetof(Vertex, position)},
        {"A", 1, Format::RGB_Float32, offsetof(Vertex, color)},
        {"A", 2, Format::RG_Float32, offsetof(Vertex, uv)},
    };

    m_inputLayout = renderer->createInputLayout(inputElements, SLANG_COUNT_OF(inputElements));
    if (!m_inputLayout)
        return SLANG_FAIL;

    BufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(kVertexCount * sizeof(Vertex));

    m_vertexBuffer = renderer->createBufferResource(
        Resource::Usage::VertexBuffer, vertexBufferDesc, kVertexData);
    if (!m_vertexBuffer)
        return SLANG_FAIL;

    {
        switch (m_options.shaderType)
        {
        default:
            assert(!"unexpected test shader type");
            return SLANG_FAIL;

        case Options::ShaderProgramType::Compute:
            {
                ComputePipelineStateDesc desc;
                desc.pipelineLayout = m_bindingState->pipelineLayout;
                desc.program = m_shaderProgram;

                m_pipelineState = renderer->createComputePipelineState(desc);
            }
            break;

        case Options::ShaderProgramType::Graphics:
        case Options::ShaderProgramType::GraphicsCompute:
            {
                GraphicsPipelineStateDesc desc;
                desc.pipelineLayout = m_bindingState->pipelineLayout;
                desc.program = m_shaderProgram;
                desc.inputLayout = m_inputLayout;
                desc.renderTargetCount = m_bindingState->m_numRenderTargets;

                m_pipelineState = renderer->createGraphicsPipelineState(desc);
            }
            break;
        }
    }

    // If success must have a pipeline state
    return m_pipelineState ? SLANG_OK : SLANG_FAIL;
}

SlangResult ShaderObjectRenderTestApp::initialize(
    SlangSession* session,
    Renderer* renderer,
    const Options& options,
    const ShaderCompilerUtil::Input& input)
{
    m_options = options;

    // We begin by compiling the shader file and entry points that specified via the options.
    //
    SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, options, input, m_compilationOutput));
    m_shaderInputLayout = m_compilationOutput.layout;

    // Once the shaders have been compiled we load them via the underlying API.
    //
    SLANG_RETURN_ON_FAIL(renderer->createProgram(m_compilationOutput.output.desc, m_shaderProgram.writeRef()));

    // If we are doing a non-pass-through compilation, then we will rely on
    // Slang's reflection API to tell us what the parameters of the program are.
    //
    RefPtr<ProgramLayout> programLayout = nullptr;

    // Okay, we will use Slang reflection to determine what the parameters of the shader are.
    //
    auto slangReflection = (slang::ProgramLayout*) spGetReflection(m_compilationOutput.output.getRequestForReflection());
    {
        ProgramLayout::Builder builder(renderer);
        builder.addGlobalParams(slangReflection->getGlobalParamsVarLayout());

        // TODO: Also need to reflect entry points here.

        SlangInt entryPointCount = slangReflection->getEntryPointCount();
        for(SlangInt e = 0; e < entryPointCount; ++e)
        {
            auto slangEntryPoint = slangReflection->getEntryPointByIndex(e);

            EntryPointLayout::Builder entryPointBuilder(renderer);
            entryPointBuilder.addEntryPointParams(slangEntryPoint);

            RefPtr<EntryPointLayout> entryPointLayout;
            SLANG_RETURN_ON_FAIL(entryPointBuilder.build(entryPointLayout.writeRef()));

            builder.addEntryPoint(entryPointLayout);
        }

        SLANG_RETURN_ON_FAIL(builder.build(programLayout.writeRef()));
    }

    // The shape of the parameters will determine the pipeline layout that we use.
    //
    RefPtr<PipelineLayout> pipelineLayout = programLayout->getPipelineLayout();

    // Once we have determined the layout of all the parameters we need to bind,
    // we will create a shader object to use for storing and binding those parameters.
    //
    RefPtr<ProgramVars> programVars;
    SLANG_RETURN_ON_FAIL(ProgramVars::create(renderer, programLayout, programVars.writeRef()));
    m_programVars = programVars;

    // Now we need to assign from the input parameter data that was parsed into
    // the program vars we allocated.
    //
    SLANG_RETURN_ON_FAIL(_assignVarsFromLayout(renderer, programVars, m_compilationOutput.layout, m_outputPlan, slangReflection));

	m_renderer = renderer;

    // TODO(tfoley): use each API's reflection interface to query the constant-buffer size needed
//    m_constantBufferSize = 16 * sizeof(float);

    {
        switch(m_options.shaderType)
        {
        default:
            assert(!"unexpected test shader type");
            return SLANG_FAIL;

        case Options::ShaderProgramType::Compute:
            {
                ComputePipelineStateDesc desc;
                desc.pipelineLayout = pipelineLayout;
                desc.program = m_shaderProgram;

                m_pipelineState = renderer->createComputePipelineState(desc);
            }
            break;

        case Options::ShaderProgramType::Graphics:
        case Options::ShaderProgramType::GraphicsCompute:
            {
                // TODO: We should conceivably be able to match up the "available" vertex
                // attributes, as defined by the vertex stream(s) on the model being
                // renderer, with the "required" vertex attributes as defiend on the
                // shader.
                //
                // For now we just create a fixed input layout for all graphics tests
                // since at present they all draw the same single triangle with a
                // fixed/known set of attributes.
                //
                const InputElementDesc inputElements[] = {
                    { "A", 0, Format::RGB_Float32, offsetof(Vertex, position) },
                    { "A", 1, Format::RGB_Float32, offsetof(Vertex, color) },
                    { "A", 2, Format::RG_Float32,  offsetof(Vertex, uv) },
                };

                RefPtr<InputLayout> inputLayout;
                SLANG_RETURN_ON_FAIL(renderer->createInputLayout(inputElements, SLANG_COUNT_OF(inputElements), inputLayout.writeRef()));

                BufferResource::Desc vertexBufferDesc;
                vertexBufferDesc.init(kVertexCount * sizeof(Vertex));

                SLANG_RETURN_ON_FAIL(renderer->createBufferResource(Resource::Usage::VertexBuffer, vertexBufferDesc, kVertexData, m_vertexBuffer.writeRef()));

                GraphicsPipelineStateDesc desc;
                desc.pipelineLayout = pipelineLayout;
                desc.program = m_shaderProgram;
                desc.inputLayout = inputLayout;
                desc.renderTargetCount = programLayout->getRenderTargetCount();

                m_pipelineState = renderer->createGraphicsPipelineState(desc);
            }
            break;
        }
    }

    // If success must have a pipeline state
    return m_pipelineState ? SLANG_OK : SLANG_FAIL;
}

Result RenderTestApp::_initializeShaders(SlangSession* session, Renderer* renderer, Options::ShaderProgramType shaderType, const ShaderCompilerUtil::Input& input)
{
    SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, m_options, input,  m_compilationOutput));
    m_shaderInputLayout = m_compilationOutput.layout;
    m_shaderProgram = renderer->createProgram(m_compilationOutput.output.desc);
    return m_shaderProgram ? SLANG_OK : SLANG_FAIL;
}


void LegacyRenderTestApp::setProjectionMatrix()
{
    auto mappedData = m_renderer->map(m_constantBuffer, MapFlavor::WriteDiscard);
    if (mappedData)
    {
        const ProjectionStyle projectionStyle =
            RendererUtil::getProjectionStyle(m_renderer->getRendererType());
        RendererUtil::getIdentityProjection(projectionStyle, (float*)mappedData);

        m_renderer->unmap(m_constantBuffer);
    }
}

void ShaderObjectRenderTestApp::setProjectionMatrix()
{
    const ProjectionStyle projectionStyle =
        RendererUtil::getProjectionStyle(m_renderer->getRendererType());

    float projectionMatrix[16];
    RendererUtil::getIdentityProjection(projectionStyle, projectionMatrix);
    ShaderCursor(m_programVars)
        .getField("Uniforms")
        .getDereferenced()
        .setData(projectionMatrix, sizeof(projectionMatrix));
}

void RenderTestApp::renderFrame()
{
    setProjectionMatrix();

    auto pipelineType = PipelineType::Graphics;

    m_renderer->setPipelineState(pipelineType, m_pipelineState);

	m_renderer->setPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_renderer->setVertexBuffer(0, m_vertexBuffer, sizeof(Vertex));

    applyBinding(pipelineType);

	m_renderer->draw(3);
}

void RenderTestApp::runCompute()
{
    auto pipelineType = PipelineType::Compute;
    m_renderer->setPipelineState(pipelineType, m_pipelineState);
    applyBinding(pipelineType);

    m_startTicks = ProcessUtil::getClockTick();

	m_renderer->dispatchCompute(m_options.computeDispatchSize[0], m_options.computeDispatchSize[1], m_options.computeDispatchSize[2]);
}

void RenderTestApp::finalize()
{
}

Result LegacyRenderTestApp::writeBindingOutput(BindRoot* bindRoot, const char* fileName)
{
    // Submit the work
    m_renderer->submitGpuWork();
    // Wait until everything is complete
    m_renderer->waitForGpu();

    FILE * f = fopen(fileName, "wb");
    if (!f)
    {
        return SLANG_FAIL;
    }
    FileWriter writer(f, WriterFlags(0));

    for(auto binding : m_bindingState->outputBindings)
    {
        auto i = binding.entryIndex;
        const auto& layoutBinding = m_shaderInputLayout.entries[i];

        assert(layoutBinding.isOutput);
        
        if (binding.resource && binding.resource->isBuffer())
        {
            BufferResource* bufferResource = static_cast<BufferResource*>(binding.resource.Ptr());
            const size_t bufferSize = bufferResource->getDesc().sizeInBytes;

            unsigned int* ptr = (unsigned int*)m_renderer->map(bufferResource, MapFlavor::HostRead);
            if (!ptr)
            {
                return SLANG_FAIL;
            }

            const SlangResult res = ShaderInputLayout::writeBinding(bindRoot, m_shaderInputLayout.entries[i], ptr, bufferSize, &writer);

            m_renderer->unmap(bufferResource);

            SLANG_RETURN_ON_FAIL(res);
        }
        else
        {
            printf("invalid output type at %d.\n", int(i));
        }
    }
    
    return SLANG_OK;
}

Result ShaderObjectRenderTestApp::writeBindingOutput(BindRoot* bindRoot, const char* fileName)
{
    // Submit the work
    m_renderer->submitGpuWork();
    // Wait until everything is complete
    m_renderer->waitForGpu();

    FILE * f = fopen(fileName, "wb");
    if (!f)
    {
        return SLANG_FAIL;
    }
    FileWriter writer(f, WriterFlags(0));

    for(auto outputItem : m_outputPlan.items)
    {
        auto& inputEntry = m_shaderInputLayout.entries[outputItem.inputLayoutEntryIndex];
        assert(inputEntry.isOutput);

        auto resource = outputItem.resource;
        if (resource && resource->isBuffer())
        {
            BufferResource* bufferResource = static_cast<BufferResource*>(resource.Ptr());
            const size_t bufferSize = bufferResource->getDesc().sizeInBytes;

            unsigned int* ptr = (unsigned int*)m_renderer->map(bufferResource, MapFlavor::HostRead);
            if (!ptr)
            {
                return SLANG_FAIL;
            }

            const SlangResult res = ShaderInputLayout::writeBinding(bindRoot, inputEntry, ptr, bufferSize, &writer);

            m_renderer->unmap(bufferResource);

            SLANG_RETURN_ON_FAIL(res);
        }
        else
        {
            printf("invalid output type at %d.\n", int(outputItem.inputLayoutEntryIndex));
        }
    }
    return SLANG_OK;
}


Result RenderTestApp::writeScreen(const char* filename)
{
    Surface surface;
    SLANG_RETURN_ON_FAIL(m_renderer->captureScreenSurface(surface));
    return PngSerializeUtil::write(filename, surface);
}

Result RenderTestApp::update(Window* window)
{
    // Whenever we don't have Windows events to process, we render a frame.
    if (m_options.shaderType == Options::ShaderProgramType::Compute)
    {
        runCompute();
    }
    else
    {
        static const float kClearColor[] = { 0.25, 0.25, 0.25, 1.0 };
        m_renderer->setClearColor(kClearColor);
        m_renderer->clearFrame();

        renderFrame();
    }

    // If we are in a mode where output is requested, we need to snapshot the back buffer here
    if (m_options.outputPath || m_options.performanceProfile)
    {
        // Submit the work
        m_renderer->submitGpuWork();
        // Wait until everything is complete
        m_renderer->waitForGpu();

        if (m_options.performanceProfile)
        {
#if 0
            // It might not be enough on some APIs to 'waitForGpu' to mean the computation has completed. Let's lock an output
            // buffer to be sure
            if (m_bindingState->outputBindings.getCount() > 0)
            {
                const auto& binding = m_bindingState->outputBindings[0];
                auto i = binding.entryIndex;
                const auto& layoutBinding = m_shaderInputLayout.entries[i];

                assert(layoutBinding.isOutput);
                
                if (binding.resource && binding.resource->isBuffer())
                {
                    BufferResource* bufferResource = static_cast<BufferResource*>(binding.resource.Ptr());
                    const size_t bufferSize = bufferResource->getDesc().sizeInBytes;
                    unsigned int* ptr = (unsigned int*)m_renderer->map(bufferResource, MapFlavor::HostRead);
                    if (!ptr)
                    {                            
                        return SLANG_FAIL;
                    }
                    m_renderer->unmap(bufferResource);
                }
            }
#endif

            // Note we don't do the same with screen rendering -> as that will do a lot of work, which may swamp any computation
            // so can only really profile compute shaders at the moment

            const uint64_t endTicks = ProcessUtil::getClockTick();

            _outputProfileTime(m_startTicks, endTicks);
        }

        if (m_options.outputPath)
        {
            if (m_options.shaderType == Options::ShaderProgramType::Compute || m_options.shaderType == Options::ShaderProgramType::GraphicsCompute)
            {
                auto request = m_compilationOutput.output.getRequestForReflection();
                auto slangReflection = (slang::ShaderReflection*) spGetReflection(request);

                BindSet bindSet;
                GPULikeBindRoot bindRoot;
                bindRoot.init(&bindSet, slangReflection, 0);

                BindRoot* outputBindRoot = m_options.outputUsingType ? &bindRoot : nullptr;

                SLANG_RETURN_ON_FAIL(writeBindingOutput(outputBindRoot, m_options.outputPath));
            }
            else
            {
                SlangResult res = writeScreen(m_options.outputPath);
                if (SLANG_FAILED(res))
                {
                    fprintf(stderr, "ERROR: failed to write screen capture to file\n");
                    return res;
                }
            }
        }
        // We are done
        window->postQuit();
        return SLANG_OK;
    }

    m_renderer->presentFrame();
    return SLANG_OK;
}


static SlangResult _setSessionPrelude(const Options& options, const char* exePath, SlangSession* session)
{
    // Let's see if we need to set up special prelude for HLSL
    if (options.nvapiExtnSlot.getLength())
    {
        String rootPath;
        SLANG_RETURN_ON_FAIL(TestToolUtil::getRootPath(exePath, rootPath));

        String includePath;
        SLANG_RETURN_ON_FAIL(TestToolUtil::getIncludePath(rootPath, "external/nvapi/nvHLSLExtns.h", includePath));

        StringBuilder buf;
        // We have to choose a slot that NVAPI will use. 
        buf << "#define NV_SHADER_EXTN_SLOT " << options.nvapiExtnSlot << "\n";

        // Include the NVAPI header
        buf << "#include \"" << includePath << "\"\n\n";

        session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, buf.getBuffer());
    }
    else
    {
        session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, "");
    }

    return SLANG_OK;
}

} //  namespace renderer_test

static SlangResult _innerMain(Slang::StdWriters* stdWriters, SlangSession* session, int argcIn, const char*const* argvIn)
{
    using namespace renderer_test;
    using namespace Slang;

    StdWriters::setSingleton(stdWriters);

    Options options;

	// Parse command-line options
	SLANG_RETURN_ON_FAIL(Options::parse(argcIn, argvIn, StdWriters::getError(), options));

    // Declare window pointer before renderer, such that window is released after renderer
    RefPtr<renderer_test::Window> window;

    ShaderCompilerUtil::Input input;
    
    input.profile = "";
    input.target = SLANG_TARGET_NONE;
    input.args = &options.slangArgs[0];
    input.argCount = options.slangArgCount;

	SlangSourceLanguage nativeLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;
	SlangPassThrough slangPassThrough = SLANG_PASS_THROUGH_NONE;
    char const* profileName = "";
	switch (options.rendererType)
	{
		case RendererType::DirectX11:
			input.target = SLANG_DXBC;
            input.profile = "sm_5_0";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            slangPassThrough = SLANG_PASS_THROUGH_FXC;
            
			break;

		case RendererType::DirectX12:
			input.target = SLANG_DXBC;
            input.profile = "sm_5_0";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            slangPassThrough = SLANG_PASS_THROUGH_FXC;
            
            if( options.useDXIL )
            {
                input.target = SLANG_DXIL;
                input.profile = "sm_6_0";
                slangPassThrough = SLANG_PASS_THROUGH_DXC;
            }
			break;

		case RendererType::OpenGl:
			input.target = SLANG_GLSL;
            input.profile = "glsl_430";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            slangPassThrough = SLANG_PASS_THROUGH_GLSLANG;
			break;

		case RendererType::Vulkan:
			input.target = SLANG_SPIRV;
            input.profile = "glsl_430";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            slangPassThrough = SLANG_PASS_THROUGH_GLSLANG;
			break;
        case RendererType::CPU:
            input.target = SLANG_HOST_CALLABLE;
            input.profile = "";
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CPP;
            slangPassThrough = SLANG_PASS_THROUGH_GENERIC_C_CPP;
            break;
        case RendererType::CUDA:
            input.target = SLANG_PTX;
            input.profile = "";
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CUDA;
            slangPassThrough = SLANG_PASS_THROUGH_NVRTC;
            break;

		default:
			fprintf(stderr, "error: unexpected\n");
			return SLANG_FAIL;
	}

    switch (options.inputLanguageID)
    {
        case Options::InputLanguageID::Slang:
            input.sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
            input.passThrough = SLANG_PASS_THROUGH_NONE;
            break;

        case Options::InputLanguageID::Native:
            input.sourceLanguage = nativeLanguage;
            input.passThrough = slangPassThrough;
            break;

        default:
            break;
    }

    switch( options.shaderType )
    {
    case Options::ShaderProgramType::Graphics:
    case Options::ShaderProgramType::GraphicsCompute:
        input.pipelineType = PipelineType::Graphics;
        break;

    case Options::ShaderProgramType::Compute:
        input.pipelineType = PipelineType::Compute;
        break;

    case Options::ShaderProgramType::RayTracing:
        input.pipelineType = PipelineType::RayTracing;
        break;

    default:
        break;
    }

    if (options.sourceLanguage != SLANG_SOURCE_LANGUAGE_UNKNOWN)
    {
        input.sourceLanguage = options.sourceLanguage;

        if (input.sourceLanguage == SLANG_SOURCE_LANGUAGE_C || input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP)
        {
            input.passThrough = SLANG_PASS_THROUGH_GENERIC_C_CPP;
        }
    }

    // Use the profile name set on options if set
    input.profile = options.profileName ? options.profileName : input.profile;

    StringBuilder rendererName;
    rendererName << "[" << RendererUtil::toText(options.rendererType) << "] ";
    if (options.adapter.getLength())
    {
        rendererName << "'" << options.adapter << "'";
    }

    if (options.onlyStartup)
    {
        switch (options.rendererType)
        {
            case RendererType::CUDA:
            {
#if RENDER_TEST_CUDA
                return SLANG_SUCCEEDED(spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_NVRTC)) && CUDAComputeUtil::canCreateDevice() ? SLANG_OK : SLANG_FAIL;
#else
                return SLANG_FAIL;
#endif
            }
            case RendererType::CPU:
            {
                // As long as we have CPU, then this should work
                return spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_GENERIC_C_CPP);
            }
            default: break;
        }
    }

    Index nvapiExtnSlot = -1;

    // Let's see if we need to set up special prelude for HLSL
    if (options.nvapiExtnSlot.getLength() && options.nvapiExtnSlot[0] == 'u')
    {
        //
        Slang::Int value;
        UnownedStringSlice slice = options.nvapiExtnSlot.getUnownedSlice();
        UnownedStringSlice indexText(slice.begin() + 1 , slice.end());
        if (SLANG_SUCCEEDED(StringUtil::parseInt(indexText, value)))
        {
            nvapiExtnSlot = Index(value);
        }
    }

    // If can't set up a necessary prelude make not available (which will lead to the test being ignored)
    if (SLANG_FAILED(_setSessionPrelude(options, argvIn[0], session)))
    {
        return SLANG_E_NOT_AVAILABLE;
    }

    // If it's CPU testing we don't need a window or a renderer
    if (options.rendererType == RendererType::CPU)
    {
        // Check we have all the required features
        for (const auto& renderFeature : options.renderFeatures)
        {
            if (!CPUComputeUtil::hasFeature(renderFeature.getUnownedSlice()))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }

        ShaderCompilerUtil::OutputAndLayout compilationAndLayout;
        SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, options, input, compilationAndLayout));

        {
            // Get the shared library -> it contains the executable code, we need to keep around if we recompile
            ComPtr<ISlangSharedLibrary> sharedLibrary;
            SLANG_RETURN_ON_FAIL(spGetEntryPointHostCallable(compilationAndLayout.output.getRequestForKernels(), 0, 0, sharedLibrary.writeRef()));

            // This is a hack to work around, reflection when compiling straight C/C++ code. In that case the code is just passed
            // straight through to the C++ compiler so no reflection. In these tests though we should have conditional code
            // (performance-profile.slang for example), such that there is both a slang and C++ code, and it is the job
            // of the test implementer to *ensure* that the straight C++ code has the same layout as the slang C++ backend.
            //
            // If we are running c/c++ we still need binding information, so compile again as slang source
            if (options.sourceLanguage == SLANG_SOURCE_LANGUAGE_C || input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP)
            {
                ShaderCompilerUtil::Input slangInput = input;
                slangInput.sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
                slangInput.passThrough = SLANG_PASS_THROUGH_NONE;
                // We just want CPP, so we get suitable reflection
                slangInput.target = SLANG_CPP_SOURCE;

                SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, options, slangInput, compilationAndLayout));
            }

            // calculate binding
            CPUComputeUtil::Context context;
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::createBindlessResources(compilationAndLayout, context));
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::fillRuntimeHandleInBuffers(compilationAndLayout, context, sharedLibrary.get()));
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::calcBindings(compilationAndLayout, context));

            // Get the execution info from the lib
            CPUComputeUtil::ExecuteInfo info;
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::calcExecuteInfo(CPUComputeUtil::ExecuteStyle::GroupRange, sharedLibrary, options.computeDispatchSize, compilationAndLayout, context, info));

            const uint64_t startTicks = ProcessUtil::getClockTick();

            SLANG_RETURN_ON_FAIL(CPUComputeUtil::execute(info));

            if (options.performanceProfile)
            {
                const uint64_t endTicks = ProcessUtil::getClockTick();
                _outputProfileTime(startTicks, endTicks);
            }

            if (options.outputPath)
            {
                BindRoot* outputBindRoot = options.outputUsingType ? &context.m_bindRoot : nullptr;


                // Dump everything out that was written
                SLANG_RETURN_ON_FAIL(ShaderInputLayout::writeBindings(outputBindRoot, compilationAndLayout.layout, context.m_buffers, options.outputPath));

                // Check all execution styles produce the same result
                SLANG_RETURN_ON_FAIL(CPUComputeUtil::checkStyleConsistency(sharedLibrary, options.computeDispatchSize, compilationAndLayout));
            }
        }

        return SLANG_OK;
    }

    if (options.rendererType == RendererType::CUDA)
    {        
#if RENDER_TEST_CUDA
        // Check we have all the required features
        for (const auto& renderFeature : options.renderFeatures)
        {
            if (!CUDAComputeUtil::hasFeature(renderFeature.getUnownedSlice()))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }

        ShaderCompilerUtil::OutputAndLayout compilationAndLayout;
        SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, options, input, compilationAndLayout));

        const uint64_t startTicks = ProcessUtil::getClockTick();

        CUDAComputeUtil::Context context;
        SLANG_RETURN_ON_FAIL(CUDAComputeUtil::execute(compilationAndLayout, options.computeDispatchSize, context));

        if (options.performanceProfile)
        {
            const uint64_t endTicks = ProcessUtil::getClockTick();
            _outputProfileTime(startTicks, endTicks);
        }

        if (options.outputPath)
        {
            BindRoot* outputBindRoot = options.outputUsingType ? &context.m_bindRoot : nullptr;

            // Dump everything out that was written
            SLANG_RETURN_ON_FAIL(ShaderInputLayout::writeBindings(outputBindRoot, compilationAndLayout.layout, context.m_buffers, options.outputPath));
        }

        return SLANG_OK;
#else
        return SLANG_FAIL;
#endif
    }

    Slang::RefPtr<Renderer> renderer;
    {
        RendererUtil::CreateFunc createFunc = RendererUtil::getCreateFunc(options.rendererType);
        if (createFunc)
        {
            renderer = createFunc();
        }

        if (!renderer)
        {
            if (!options.onlyStartup)
            {
                fprintf(stderr, "Unable to create renderer %s\n", rendererName.getBuffer());
            }
            return SLANG_FAIL;
        }

        Renderer::Desc desc;
        desc.width = gWindowWidth;
        desc.height = gWindowHeight;
        desc.adapter = options.adapter;
        desc.requiredFeatures = options.renderFeatures;
        desc.nvapiExtnSlot = int(nvapiExtnSlot);

        window = renderer_test::Window::create();
        SLANG_RETURN_ON_FAIL(window->initialize(gWindowWidth, gWindowHeight));

        SlangResult res = renderer->initialize(desc, window->getHandle());
        if (SLANG_FAILED(res))
        {
            // Returns E_NOT_AVAILABLE only when specified features are not available.
            // Will cause to be ignored.
            if (!options.onlyStartup && res != SLANG_E_NOT_AVAILABLE)
            {
                fprintf(stderr, "Unable to initialize renderer %s\n", rendererName.getBuffer());
            }
            return res;
        }

        for (const auto& feature : options.renderFeatures)
        {
            // If doesn't have required feature... we have to give up
            if (!renderer->hasFeature(feature.getUnownedSlice()))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }
    }
   
    // If the only test is we can startup, then we are done
    if (options.onlyStartup)
    {
        return SLANG_OK;
    }

	{
        RefPtr<RenderTestApp> app;
        if (options.useShaderObjects)
            app = new ShaderObjectRenderTestApp();
        else
            app = new LegacyRenderTestApp();
		SLANG_RETURN_ON_FAIL(app->initialize(session, renderer, options, input));
        window->show();
        return window->runLoop(app);
	}
}

SLANG_TEST_TOOL_API SlangResult innerMain(Slang::StdWriters* stdWriters, SlangSession* sharedSession, int inArgc, const char*const* inArgv)
{
    using namespace Slang;

    // Assume we will used the shared session
    ComPtr<slang::IGlobalSession> session(sharedSession);

    // The sharedSession always has a pre-loaded stdlib.
    // This differed test checks if the command line has an option to setup the stdlib.
    // If so we *don't* use the sharedSession, and create a new stdlib-less session just for this compilation. 
    if (TestToolUtil::hasDeferredStdLib(Index(inArgc - 1), inArgv + 1))
    {
        SLANG_RETURN_ON_FAIL(slang_createGlobalSessionWithoutStdLib(SLANG_API_VERSION, session.writeRef()));
    }

    SlangResult res = SLANG_FAIL;
    try
    {
        res = _innerMain(stdWriters, session, inArgc, inArgv);
    }
    catch (const Slang::Exception& exception)
    {
        stdWriters->getOut().put(exception.Message.getUnownedSlice());
        return SLANG_FAIL;
    }
    catch (...)
    {
        stdWriters->getOut().put(UnownedStringSlice::fromLiteral("Unhandled exception"));
        return SLANG_FAIL;
    }

    return res;
}

int main(int argc, char**  argv)
{
    using namespace Slang;
    SlangSession* session = spCreateSession(nullptr);

    TestToolUtil::setSessionDefaultPreludeFromExePath(argv[0], session);
    
    auto stdWriters = StdWriters::initDefaultSingleton();
    
    SlangResult res = innerMain(stdWriters, session, argc, argv);
    spDestroySession(session);

	return (int)TestToolUtil::getReturnCode(res);
}


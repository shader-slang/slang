// unit-test-binding-range-descriptor-set-reflection.cpp

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Verify the `printBindingRanges` example in the "Binding Ranges and
// Descriptor Sets" section of `docs/user-guide/09-reflection.md`.
//
// `printBindingRanges` below is a copy of the code in that section. The
// only difference is that `print` appends to a string, so the test can
// check the result. If you change one, change the other.

namespace
{
StringBuilder gOutput;

void print(const char* text)
{
    gOutput << text;
}

void print(size_t value)
{
    gOutput << (Int)value;
}

void printBindingRanges(slang::TypeLayoutReflection* typeLayout)
{
    int bindingRangeCount = typeLayout->getBindingRangeCount();
    for (int r = 0; r < bindingRangeCount; ++r)
    {
        // A binding range without descriptor ranges has no
        // descriptor set. Skip it.
        int rangeCount = typeLayout->getBindingRangeDescriptorRangeCount(r);
        if (rangeCount == 0)
            continue;

        int setIndex = typeLayout->getBindingRangeDescriptorSetIndex(r);
        int firstRangeIndex = typeLayout->getBindingRangeFirstDescriptorRangeIndex(r);

        // The space offset is the Vulkan descriptor set, the D3D12
        // register space, or the WebGPU bind group.
        size_t space = typeLayout->getDescriptorSetSpaceOffset(setIndex);

        for (int i = 0; i < rangeCount; ++i)
        {
            size_t binding = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(
                setIndex,
                firstRangeIndex + i);

            print("binding range ");
            print(r);
            print(": space ");
            print(space);
            print(", binding ");
            print(binding);
            print("\n");
        }
    }
}
} // namespace

SLANG_UNIT_TEST(bindingRangeDescriptorSetReflection)
{
    // `material` is a parameter block, so the compiler gives it its own
    // space. `output` and `other` are explicit global resources.
    const char* userSource = R"(
        struct Material
        {
            float4 tint;
            Texture2D albedo;
            SamplerState sampler;
        }

        ParameterBlock<Material> material;
        RWStructuredBuffer<uint> output : register(u2, space0);
        RWStructuredBuffer<uint> other : register(u5, space3);

        [Shader("compute")]
        [NumThreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            output[tid.x] = uint(material.tint.x + material.albedo.SampleLevel(material.sampler, float2(0, 0), 0).x);
            other[tid.x] = 0;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "test",
        "test.slang",
        userSource,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK(programLayout != nullptr);

    auto globalParamsLayout = programLayout->getGlobalParamsVarLayout()->getTypeLayout();
    SLANG_CHECK(globalParamsLayout != nullptr);

    // Binding range 0 is `material`. A parameter block has no descriptor
    // range in the global scope, so the example skips it. The other
    // binding ranges are `output` and `other`.
    gOutput.clear();
    printBindingRanges(globalParamsLayout);
    SLANG_CHECK(
        gOutput.produceString() == "binding range 1: space 0, binding 2\n"
                                   "binding range 2: space 3, binding 5\n");

    // The binding ranges inside the parameter block are relative to the
    // block. The space of the block itself is the `SubElementRegisterSpace`
    // offset of its variable layout. Binding 0 of the block holds the
    // constant buffer for `tint`, so the texture and the sampler are at
    // bindings 1 and 2. The cumulative locations are therefore space 1,
    // bindings 1 and 2.
    auto materialLayout = globalParamsLayout->getFieldByIndex(0);
    SLANG_CHECK(
        materialLayout->getTypeLayout()->getKind() == slang::TypeReflection::Kind::ParameterBlock);
    SLANG_CHECK(materialLayout->getOffset(slang::ParameterCategory::SubElementRegisterSpace) == 1);

    gOutput.clear();
    printBindingRanges(materialLayout->getTypeLayout()->getElementTypeLayout());
    SLANG_CHECK(
        gOutput.produceString() == "binding range 0: space 0, binding 1\n"
                                   "binding range 1: space 0, binding 2\n");
}

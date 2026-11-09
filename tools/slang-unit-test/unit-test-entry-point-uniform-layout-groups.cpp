#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static bool hasSpirvMemberOffset(slang::IBlob* code, uint32_t offset);

/// Check shared shader exports with distinct layouts, including layout-module cleanup.
static void checkEntryPointShaderRecordLayoutConstraints(bool obfuscate)
{
    ComPtr<slang::IGlobalSession> global;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, global.writeRef())));
    slang::TargetDesc target;
    target.format = SLANG_SPIRV;
    slang::SessionDesc desc;
    slang::CompilerOptionEntry obfuscationOption = {};
    obfuscationOption.name = slang::CompilerOptionName::Obfuscate;
    obfuscationOption.value.kind = slang::CompilerOptionValueKind::Int;
    obfuscationOption.value.intValue0 = obfuscate ? 1 : 0;
    desc.compilerOptionEntries = &obfuscationOption;
    desc.compilerOptionEntryCount = 1;
    desc.targets = &target;
    desc.targetCount = 1;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(global->createSession(desc, session.writeRef())));
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "recordGroups",
        "recordGroups.slang",
        R"(
        [raypayload] struct Payload
        {
            uint value : read(caller, closesthit, anyhit) : write(caller, closesthit, anyhit);
        }
        struct Attributes { float2 uv; }
        [shader("intersection")]
        void intersection(uniform float distance) { ReportHit(distance, 0, Attributes(float2(0))); }
        [shader("anyhit")]
        void anyHit(inout Payload payload, in Attributes attr, uniform float4 threshold)
        { if (payload.value < threshold.x) IgnoreHit(); }
        [shader("closesthit")]
        void closestHit(inout Payload payload, in Attributes attr, uniform uint bias)
        { payload.value += bias; }
    )",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module);
    ComPtr<slang::IEntryPoint> entries[3];
    const char* names[] = {"closestHit", "anyHit", "intersection"};
    slang::IComponentType* parts[] = {module, nullptr, nullptr, nullptr, nullptr};
    for (int i = 0; i < 3; ++i)
    {
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(module->findEntryPointByName(names[i], entries[i].writeRef())));
        parts[i + 1] = entries[i];
    }
    // The same source intersection function appears in a second group whose record begins
    // at a different offset. Reflection and emitted code must agree for both occurrences.
    ComPtr<slang::IComponentType> intersectionOnly;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        entries[2]->renameEntryPoint("intersectionOnly", intersectionOnly.writeRef())));
    parts[4] = intersectionOnly;
    ComPtr<slang::IComponentType> composite, constraints, constrained, linked;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(session->createCompositeComponentType(parts, 5, composite.writeRef())));
    auto legacy = composite->getLayout();
    SLANG_CHECK_ABORT(legacy);
    SLANG_CHECK(legacy->getEntryPointByIndex(1)->getShaderRecordOffset() == -1);
    ComPtr<slang::IEntryPointUniformLayoutService_Experimental> service;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(composite->queryInterface(SLANG_IID_PPV_ARGS(service.writeRef()))));
    SlangInt indices[] = {0, 1, 2};
    SlangInt onlyIndex[] = {3};
    slang::EntryPointUniformLayoutGroup groups[] = {{indices, 3}, {onlyIndex, 1}};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(service->createEntryPointUniformLayoutConstraints(
        groups,
        2,
        constraints.writeRef(),
        diagnostics.writeRef())));
    slang::IComponentType* constrainedParts[] = {composite, constraints};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        constrainedParts,
        2,
        constrained.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(constrained->link(linked.writeRef(), diagnostics.writeRef())));
    auto layout = linked->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(layout);
    SLANG_CHECK(layout->getEntryPointByIndex(0)->getShaderRecordOffset() == 0);
    SLANG_CHECK(layout->getEntryPointByIndex(1)->getShaderRecordOffset() == 16);
    SLANG_CHECK(layout->getEntryPointByIndex(2)->getShaderRecordOffset() == 4);
    SLANG_CHECK(layout->getEntryPointByIndex(3)->getShaderRecordOffset() == 0);
    SLANG_CHECK(layout->getEntryPointUniformLayoutGroupSize(0) == 32);
    for (int i = 0; i < 4; ++i)
    {
        auto entry = layout->getEntryPointByIndex(i);
        SLANG_CHECK(entry->getPushConstantOffset() == -1);
        auto parameter = entry->getParameterByIndex(i >= 2 ? 0 : 2);
        SLANG_CHECK(parameter->getOffset() == 0);
        ComPtr<slang::IBlob> code;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            linked->getEntryPointCode(i, 0, code.writeRef(), diagnostics.writeRef())));
        SLANG_CHECK(code->getBufferSize() > 0);
        if (i >= 2)
            SLANG_CHECK(hasSpirvMemberOffset(code, i == 2 ? 4 : 0));
    }
    // Whole-program code generation must preserve both exports too, rather than updating
    // one shared function's entry-point name or parameter layout for the second occurrence.
    ComPtr<slang::IBlob> allCode;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(linked->getTargetCode(0, allCode.writeRef(), diagnostics.writeRef())));
    SLANG_CHECK(hasSpirvMemberOffset(allCode, 4));
}

SLANG_UNIT_TEST(entryPointShaderRecordLayoutConstraints)
{
    checkEntryPointShaderRecordLayoutConstraints(false);
}

SLANG_UNIT_TEST(entryPointShaderRecordLayoutConstraintsObfuscated)
{
    // Target records retain lowered layouts outside the IR use graph. Obfuscation runs DCE
    // on the layout module, so this exercises their explicit lifetime through that cleanup.
    checkEntryPointShaderRecordLayoutConstraints(true);
}

SLANG_UNIT_TEST(entryPointUniformLayoutReflectionSessions)
{
    ComPtr<slang::IGlobalSession> firstGlobal;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, firstGlobal.writeRef())));
    slang::TargetDesc target;
    target.format = SLANG_SPIRV;
    slang::SessionDesc desc;
    desc.targets = &target;
    desc.targetCount = 1;
    ComPtr<slang::ISession> firstSession;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(firstGlobal->createSession(desc, firstSession.writeRef())));
    ComPtr<slang::IBlob> diagnostics;
    auto module = firstSession->loadModuleFromSourceString(
        "reflectionOwner",
        "reflectionOwner.slang",
        R"(
        struct Capture<each T>
        {
            Tuple<expand each T> values;
            RWStructuredBuffer<float> buffer;
        }
        [shader("compute")][numthreads(1, 1, 1)]
        void main(uniform Capture<float4, float4> capture) {}
    )",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module);
    ComPtr<slang::IEntryPoint> entry;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findEntryPointByName("main", entry.writeRef())));
    slang::IComponentType* parts[] = {module, entry};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        firstSession
            ->createCompositeComponentType(parts, 2, program.writeRef(), diagnostics.writeRef())));
    auto layout = program->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(layout);
    auto captureParameter = layout->getEntryPointByIndex(0)->getParameterByIndex(0);
    auto capture = captureParameter->getTypeLayout();
    SLANG_CHECK_ABORT(layout->findTypeByName("Capture<float4, float4>"));

    // A second global session changes the ambient builder and its resolution epoch.
    // Reflection on the first session must still canonicalize its own type-pack values.
    ComPtr<slang::IGlobalSession> secondGlobal;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, secondGlobal.writeRef())));
    ComPtr<slang::ISession> secondSession;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(secondGlobal->createSession(desc, secondSession.writeRef())));
    const char* names[] = {"otherA", "otherB", "otherC"};
    for (auto name : names)
    {
        SLANG_CHECK_ABORT(secondSession->loadModuleFromSourceString(
            name,
            name,
            "struct Other { float4 value; }",
            diagnostics.writeRef()));
        SLANG_CHECK(capture->getKind() == slang::TypeReflection::Kind::Struct);
        SLANG_CHECK(capture->getType()->getKind() == slang::TypeReflection::Kind::Struct);
        SLANG_CHECK(::strcmp(capture->getType()->getName(), "Capture") == 0);
        SLANG_CHECK(captureParameter->getOffset(SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE) == 0);
        SLANG_CHECK(
            captureParameter->getBindingSpace(SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE) == 0);
        SLANG_CHECK_ABORT(layout->findTypeByName("Capture<float4, float4>"));
    }
}

/// Check the actual SPIR-V member placement, independently of the reflection API.
static bool hasSpirvMemberOffset(slang::IBlob* code, uint32_t offset)
{
    auto words = static_cast<uint32_t const*>(code->getBufferPointer());
    auto count = code->getBufferSize() / sizeof(uint32_t);
    for (size_t i = 5; i < count;)
    {
        uint32_t size = words[i] >> 16;
        if (!size || size > count - i)
            return false;
        // OpMemberDecorate(type, member, Offset, byteOffset).
        if ((words[i] & 0xffff) == 72 && size == 5 && words[i + 3] == 35 && words[i + 4] == offset)
            return true;
        i += size;
    }
    return false;
}

SLANG_UNIT_TEST(entryPointUniformLayoutConstraintComponent)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    slang::TargetDesc targets[3];
    targets[0].format = SLANG_SPIRV;
    targets[1].format = SLANG_DXIL;
    targets[2].format = SLANG_GLSL;
    slang::CompilerOptionEntry useEntryPointNames;
    useEntryPointNames.name = slang::CompilerOptionName::VulkanUseEntryPointName;
    useEntryPointNames.value.kind = slang::CompilerOptionValueKind::Int;
    useEntryPointNames.value.intValue0 = 1;
    targets[0].compilerOptionEntries = &useEntryPointNames;
    targets[0].compilerOptionEntryCount = 1;
    slang::SessionDesc desc;
    desc.targets = targets;
    desc.targetCount = 3;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createSession(desc, session.writeRef())));

    const char* source = R"(
        [shader("vertex")]
        float4 vertexMain(uniform float4x4 transform) : SV_Position { return transform[0]; }
        [shader("fragment")]
        float4 fragmentA(uniform float4 color) : SV_Target { return color; }
        [shader("fragment")]
        float4 fragmentB(uniform float4 color, uniform float4 extra) : SV_Target { return color + extra; }
        [shader("compute")][numthreads(1, 1, 1)]
        void unrelated(uniform float4 data) {}
        [shader("vertex")]
        float4 genericVertex<T>(uniform T capture) : SV_Position { return float4(0); }
    )";
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "layoutGroups",
        "layoutGroups.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module);
    ComPtr<slang::IEntryPoint> vertex, fragmentA, fragmentB;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(module->findEntryPointByName("vertexMain", vertex.writeRef())));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(module->findEntryPointByName("fragmentA", fragmentA.writeRef())));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(module->findEntryPointByName("fragmentB", fragmentB.writeRef())));
    slang::IComponentType* parts[] = {module, vertex, fragmentA, fragmentB};
    ComPtr<slang::IComponentType> shaders;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        session
            ->createCompositeComponentType(parts, 4, shaders.writeRef(), diagnostics.writeRef())));

    ComPtr<slang::IEntryPointUniformLayoutService_Experimental> service;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(shaders->queryInterface(SLANG_IID_PPV_ARGS(service.writeRef()))));
    auto defaultLayout = shaders->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(defaultLayout);
    SLANG_CHECK(defaultLayout->getEntryPointUniformLayoutGroupCount() == 0);
    SLANG_CHECK(defaultLayout->getEntryPointByIndex(1)->getPushConstantOffset() == -1);
    SLANG_CHECK(defaultLayout->getEntryPointByIndex(1)->getParameterByIndex(0)->getOffset() == 0);
    SlangInt pipelineA[] = {0, 1};
    SlangInt pipelineB[] = {0, 2};
    slang::EntryPointUniformLayoutGroup groups[] = {{pipelineA, 2}, {pipelineB, 2}};
    ComPtr<slang::IComponentType> constraints;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(service->createEntryPointUniformLayoutConstraints(
        groups,
        2,
        constraints.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK(constraints->getSpecializationParamCount() == 0);

    slang::IComponentType* batchParts[] = {shaders, constraints};
    ComPtr<slang::IComponentType> batch, linked;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        batchParts,
        2,
        batch.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK(batchParts[0] == shaders.get());
    SLANG_CHECK(batchParts[1] == constraints.get());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(batch->link(linked.writeRef(), diagnostics.writeRef())));
    auto layout = linked->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(layout);
    SLANG_CHECK(layout->getEntryPointCount() == 3);
    SLANG_CHECK(layout->getParameterCount() == 0);
    SLANG_CHECK(layout->getEntryPointUniformLayoutGroupCount() == 2);
    SLANG_CHECK(layout->getEntryPointUniformLayoutGroupEntryPointCount(1) == 2);
    SLANG_CHECK(layout->getEntryPointUniformLayoutGroupEntryPointIndex(1, 1) == 2);
    SLANG_CHECK(layout->getEntryPointByIndex(0)->getPushConstantOffset() == 0);
    SLANG_CHECK(layout->getEntryPointByIndex(1)->getPushConstantOffset() == 64);
    SLANG_CHECK(layout->getEntryPointByIndex(2)->getPushConstantOffset() == 64);
    SLANG_CHECK(layout->getEntryPointByIndex(2)->getPushConstantSize() == 32);
    SLANG_CHECK(layout->getEntryPointUniformLayoutGroupSize(0) == 80);
    SLANG_CHECK(layout->getEntryPointUniformLayoutGroupSize(1) == 96);
    SLANG_CHECK(layout->getEntryPointByIndex(2)->getParameterByIndex(1)->getOffset() == 16);

    for (SlangInt entry = 0; entry < 3; ++entry)
    {
        ComPtr<slang::IBlob> code;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            linked->getEntryPointCode(entry, 0, code.writeRef(), diagnostics.writeRef())));
        SLANG_CHECK(hasSpirvMemberOffset(code, entry == 0 ? 0 : 64));
        if (entry == 2)
            SLANG_CHECK(hasSpirvMemberOffset(code, 80));
    }
    ComPtr<slang::IBlob> wholeProgram;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(linked->getTargetCode(0, wholeProgram.writeRef(), diagnostics.writeRef())));
    SLANG_CHECK(hasSpirvMemberOffset(wholeProgram, 64));
    SLANG_CHECK(hasSpirvMemberOffset(wholeProgram, 80));
    auto d3dLayout = linked->getLayout(1, diagnostics.writeRef());
    SLANG_CHECK_ABORT(d3dLayout);
    SLANG_CHECK(d3dLayout->getEntryPointByIndex(1)->getPushConstantOffset() == -1);
    ComPtr<slang::IBlob> glslCode;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        linked->getEntryPointCode(2, 2, glslCode.writeRef(), diagnostics.writeRef())));
    SLANG_CHECK(::strstr(static_cast<const char*>(glslCode->getBufferPointer()), "offset = 64"));
    SLANG_CHECK(::strstr(static_cast<const char*>(glslCode->getBufferPointer()), "offset = 80"));

    // Ordinary compilation still uses the legacy implicit block at zero.
    ComPtr<slang::IBlob> defaultCode;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        shaders->getEntryPointCode(1, 0, defaultCode.writeRef(), diagnostics.writeRef())));
    SLANG_CHECK(hasSpirvMemberOffset(defaultCode, 0));
    SLANG_CHECK(!hasSpirvMemberOffset(defaultCode, 64));

    // Nesting shifts the source namespace without applying constraints to its neighbor.
    ComPtr<slang::IEntryPoint> unrelated;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(module->findEntryPointByName("unrelated", unrelated.writeRef())));
    slang::IComponentType* nestedParts[] = {unrelated, batch};
    ComPtr<slang::IComponentType> nested;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        nestedParts,
        2,
        nested.writeRef(),
        diagnostics.writeRef())));
    auto nestedLayout = nested->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(nestedLayout);
    SLANG_CHECK(nestedLayout->getEntryPointCount() == 4);
    SLANG_CHECK(nestedLayout->getEntryPointByIndex(0)->getPushConstantOffset() == -1);
    SLANG_CHECK(nestedLayout->getEntryPointByIndex(2)->getPushConstantOffset() == 64);
    SLANG_CHECK(nestedLayout->getEntryPointUniformLayoutGroupEntryPointIndex(0, 0) == 1);
    SLANG_CHECK(nestedLayout->getEntryPointUniformLayoutGroupEntryPointIndex(1, 1) == 3);

    // A disconnected group can start at zero without moving either graphics payload.
    ComPtr<slang::IEntryPointUniformLayoutService_Experimental> unrelatedService;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unrelated->queryInterface(SLANG_IID_PPV_ARGS(unrelatedService.writeRef()))));
    SlangInt unrelatedIndex = 0;
    slang::EntryPointUniformLayoutGroup unrelatedGroup = {&unrelatedIndex, 1};
    ComPtr<slang::IComponentType> unrelatedConstraints, disjointBatch;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(unrelatedService->createEntryPointUniformLayoutConstraints(
        &unrelatedGroup,
        1,
        unrelatedConstraints.writeRef(),
        diagnostics.writeRef())));
    slang::IComponentType* disjointParts[] = {nested, unrelatedConstraints};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        disjointParts,
        2,
        disjointBatch.writeRef(),
        diagnostics.writeRef())));
    auto disjointLayout = disjointBatch->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(disjointLayout);
    SLANG_CHECK(disjointLayout->getEntryPointByIndex(0)->getPushConstantOffset() == 0);
    SLANG_CHECK(disjointLayout->getEntryPointByIndex(2)->getPushConstantOffset() == 64);

    // The constraint is declared before T is known. Specializing the entire composite
    // preserves its source occurrence, then allocates using T's concrete payload size.
    ComPtr<slang::IEntryPoint> genericVertex;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(module->findEntryPointByName("genericVertex", genericVertex.writeRef())));
    slang::IComponentType* genericParts[] = {module, genericVertex, fragmentA};
    ComPtr<slang::IComponentType> genericShaders, genericConstraints, genericBatch;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        genericParts,
        3,
        genericShaders.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IEntryPointUniformLayoutService_Experimental> genericService;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        genericShaders->queryInterface(SLANG_IID_PPV_ARGS(genericService.writeRef()))));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(genericService->createEntryPointUniformLayoutConstraints(
        groups,
        1,
        genericConstraints.writeRef(),
        diagnostics.writeRef())));
    slang::IComponentType* genericBatchParts[] = {genericShaders, genericConstraints};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        genericBatchParts,
        2,
        genericBatch.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK(genericBatch->getSpecializationParamCount() == 1);
    slang::SpecializationArg arg = slang::SpecializationArg::fromExpr("float4x4");
    ComPtr<slang::IComponentType> specialized;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        genericBatch->specialize(&arg, 1, specialized.writeRef(), diagnostics.writeRef())));
    auto specializedLayout = specialized->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(specializedLayout);
    SLANG_CHECK(specializedLayout->getEntryPointByIndex(1)->getPushConstantOffset() == 64);

    arg = slang::SpecializationArg::fromExpr("half");
    ComPtr<slang::IComponentType> halfSpecialized;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        genericBatch->specialize(&arg, 1, halfSpecialized.writeRef(), diagnostics.writeRef())));
    auto halfLayout = halfSpecialized->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(halfLayout);
    SLANG_CHECK(
        halfLayout->getEntryPointByIndex(0)->getParameterByIndex(0)->getTypeLayout()->getSize() ==
        2);
    SLANG_CHECK(halfLayout->getEntryPointByIndex(0)->getPushConstantSize() == 4);
    SLANG_CHECK(halfLayout->getEntryPointByIndex(1)->getPushConstantOffset() == 16);

    // Reordering and repeating group members does not select a different contract.
    SlangInt repeated[] = {1, 0, 1};
    slang::EntryPointUniformLayoutGroup equivalent[] = {
        {pipelineB, 2},
        {repeated, 3},
        {pipelineA, 2}};
    ComPtr<slang::IComponentType> otherConstraints;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(service->createEntryPointUniformLayoutConstraints(
        equivalent,
        3,
        otherConstraints.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IBlob> firstHash, secondHash;
    batch->getEntryPointHash(0, 0, firstHash.writeRef());
    batchParts[1] = otherConstraints;
    ComPtr<slang::IComponentType> equivalentBatch;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        batchParts,
        2,
        equivalentBatch.writeRef(),
        diagnostics.writeRef())));
    equivalentBatch->getEntryPointHash(0, 0, secondHash.writeRef());
    SLANG_CHECK_ABORT(firstHash && secondHash);
    SLANG_CHECK(firstHash->getBufferSize() == secondHash->getBufferSize());
    SLANG_CHECK(
        ::memcmp(
            firstHash->getBufferPointer(),
            secondHash->getBufferPointer(),
            firstHash->getBufferSize()) == 0);

    // Adding the third coexistence edge makes all three payloads independent.
    SlangInt cliqueMembers[] = {0, 1, 2};
    slang::EntryPointUniformLayoutGroup cliqueGroup = {cliqueMembers, 3};
    ComPtr<slang::IComponentType> cliqueConstraints, cliqueBatch;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(service->createEntryPointUniformLayoutConstraints(
        &cliqueGroup,
        1,
        cliqueConstraints.writeRef(),
        diagnostics.writeRef())));
    slang::IComponentType* cliqueParts[] = {shaders, cliqueConstraints};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        cliqueParts,
        2,
        cliqueBatch.writeRef(),
        diagnostics.writeRef())));
    auto cliqueLayout = cliqueBatch->getLayout(0, diagnostics.writeRef());
    SLANG_CHECK_ABORT(cliqueLayout);
    SLANG_CHECK(cliqueLayout->getEntryPointByIndex(2)->getPushConstantOffset() == 80);
    cliqueBatch->getEntryPointHash(0, 0, secondHash.writeRef());
    SLANG_CHECK_ABORT(secondHash);
    SLANG_CHECK(
        ::memcmp(
            firstHash->getBufferPointer(),
            secondHash->getBufferPointer(),
            firstHash->getBufferSize()) != 0);

    // Identical child contents do not identify an occurrence. Choosing which child's
    // entry points are constrained must affect the final shader-cache key.
    ComPtr<slang::IComponentType> sameShaders;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        parts,
        4,
        sameShaders.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IEntryPointUniformLayoutService_Experimental> sameService;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(sameShaders->queryInterface(SLANG_IID_PPV_ARGS(sameService.writeRef()))));
    ComPtr<slang::IComponentType> sameConstraints;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(sameService->createEntryPointUniformLayoutConstraints(
        groups,
        2,
        sameConstraints.writeRef(),
        diagnostics.writeRef())));
    slang::IComponentType* occurrenceParts[] = {shaders, sameShaders, constraints};
    ComPtr<slang::IComponentType> firstOccurrence, secondOccurrence;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        occurrenceParts,
        3,
        firstOccurrence.writeRef(),
        diagnostics.writeRef())));
    occurrenceParts[2] = sameConstraints;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        occurrenceParts,
        3,
        secondOccurrence.writeRef(),
        diagnostics.writeRef())));
    firstOccurrence->getEntryPointHash(0, 0, firstHash.writeRef());
    secondOccurrence->getEntryPointHash(0, 0, secondHash.writeRef());
    SLANG_CHECK_ABORT(firstHash && secondHash);
    SLANG_CHECK(
        ::memcmp(
            firstHash->getBufferPointer(),
            secondHash->getBufferPointer(),
            firstHash->getBufferSize()) != 0);

    // Metadata must not silently link in its referenced shaders when they are absent.
    SLANG_CHECK(constraints->getLayout(0, diagnostics.writeRef()) == nullptr);

    slang::IComponentType* ambiguousParts[] = {shaders, shaders, constraints};
    ComPtr<slang::IComponentType> ambiguous;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        ambiguousParts,
        3,
        ambiguous.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK(ambiguous->getLayout(0, diagnostics.writeRef()) == nullptr);
    SLANG_CHECK(diagnostics && diagnostics->getBufferSize());
    SLANG_CHECK(constraints->getLayout(0, diagnostics.writeRef()) == nullptr);

    SlangInt invalidIndex[] = {3};
    slang::EntryPointUniformLayoutGroup invalid[] = {{invalidIndex, 1}};
    ComPtr<slang::IComponentType> invalidConstraints;
    SLANG_CHECK(SLANG_FAILED(service->createEntryPointUniformLayoutConstraints(
        invalid,
        1,
        invalidConstraints.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK(!invalidConstraints);

    // Explicit global push constants and independent implicit captures would require
    // one merged SPIR-V block. Diagnose that opt-in combination rather than emit two.
    auto explicitModule = session->loadModuleFromSourceString(
        "explicitLayout",
        "explicitLayout.slang",
        R"(
        struct Constants { float4 color; }
        [[vk::push_constant]] ConstantBuffer<Constants> globals;
        [shader("fragment")]
        float4 main(uniform float4 capture) : SV_Target { return globals.color + capture; }
    )",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(explicitModule);
    ComPtr<slang::IEntryPoint> explicitEntry;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(explicitModule->findEntryPointByName("main", explicitEntry.writeRef())));
    slang::IComponentType* explicitParts[] = {explicitModule, explicitEntry};
    ComPtr<slang::IComponentType> explicitShaders, explicitConstraints, explicitBatch;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        explicitParts,
        2,
        explicitShaders.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IEntryPointUniformLayoutService_Experimental> explicitService;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        explicitShaders->queryInterface(SLANG_IID_PPV_ARGS(explicitService.writeRef()))));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(explicitService->createEntryPointUniformLayoutConstraints(
        &unrelatedGroup,
        1,
        explicitConstraints.writeRef(),
        diagnostics.writeRef())));
    slang::IComponentType* explicitBatchParts[] = {explicitShaders, explicitConstraints};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        explicitBatchParts,
        2,
        explicitBatch.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK(explicitBatch->getLayout(0, diagnostics.writeRef()) == nullptr);
    SLANG_CHECK(diagnostics && diagnostics->getBufferSize());
}

// unit-test-global-constant-buffer-binding-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// The global-scope uniforms of a program are gathered into a default constant buffer.
// When an explicit binding takes slot 0, that buffer is allocated slot 1. Reflection
// offsets are relative to the parent variable, and the global-scope parameters keep
// absolute bindings, so the buffer's slot belongs on the scope's container var layout,
// with the scope var layout itself at offset 0. Walking scope + element + field then
// gives each global parameter its real binding, and scope + container gives the
// buffer's binding.
//
// See: https://github.com/shader-slang/slang/issues/14013

// Compile `source` for one target with the given session options and link it with its
// `main` entry point. The session is returned through `outSession` because the linked
// program's layout is owned by it.
static ComPtr<slang::IComponentType> _linkProgram(
    slang::IGlobalSession* globalSession,
    SlangCompileTarget format,
    const char* profile,
    slang::CompilerOptionEntry* options,
    uint32_t optionCount,
    const char* source,
    ComPtr<slang::ISession>& outSession)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = format;
    targetDesc.profile = globalSession->findProfile(profile);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = options;
    sessionDesc.compilerOptionEntryCount = optionCount;

    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, outSession.writeRef()) == SLANG_OK);
    auto session = outSession.get();

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", source, diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("main", entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);
    return linkedProgram;
}

SLANG_UNIT_TEST(globalConstantBufferBindingAfterExplicitVulkanBinding)
{
    const char* source = R"(
        [[vk::binding(0)]] RWStructuredBuffer<uint> result;
        uniform uint value;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main(uint3 tid : SV_DispatchThreadID)
        {
            result[0] = value;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    ComPtr<slang::ISession> session;
    auto program =
        _linkProgram(globalSession, SLANG_SPIRV, "spirv_1_5", nullptr, 0, source, session);

    const auto slot = slang::ParameterCategory::DescriptorTableSlot;

    auto globals = program->getLayout()->getGlobalParamsVarLayout();
    auto globalsTypeLayout = globals->getTypeLayout();
    SLANG_CHECK_ABORT(globalsTypeLayout->getKind() == slang::TypeReflection::Kind::ConstantBuffer);
    auto container = globalsTypeLayout->getContainerVarLayout();
    auto element = globalsTypeLayout->getElementVarLayout();

    SLANG_CHECK(globals->getOffset(slot) == 0);
    SLANG_CHECK(globals->getOffset(slot) + container->getOffset(slot) == 1);

    auto elementTypeLayout = element->getTypeLayout();
    SLANG_CHECK_ABORT(elementTypeLayout->getFieldCount() == 2);
    auto result = elementTypeLayout->getFieldByIndex(0);
    SLANG_CHECK(globals->getOffset(slot) + element->getOffset(slot) + result->getOffset(slot) == 0);

    // The descriptor ranges of the global-scope type are relative to the scope as well:
    // the default constant buffer at 1, `result` at 0.
    SLANG_CHECK_ABORT(globalsTypeLayout->getDescriptorSetCount() == 1);
    SlangInt rangeCount = globalsTypeLayout->getDescriptorSetDescriptorRangeCount(0);
    SLANG_CHECK(rangeCount == 2);
    for (SlangInt r = 0; r < rangeCount; ++r)
    {
        auto offset = globals->getOffset(slot) +
                      globalsTypeLayout->getDescriptorSetDescriptorRangeIndexOffset(0, r);
        switch (globalsTypeLayout->getDescriptorSetDescriptorRangeType(0, r))
        {
        case slang::BindingType::ConstantBuffer:
            SLANG_CHECK(offset == 1);
            break;
        case slang::BindingType::MutableRawBuffer:
        case slang::BindingType::MutableTypedBuffer:
            SLANG_CHECK(offset == 0);
            break;
        default:
            SLANG_CHECK(!"unexpected descriptor range");
            break;
        }
    }

    // Post-emit metadata reports the location the constant buffer is emitted at.
    ComPtr<slang::IMetadata> metadata;
    program->getTargetMetadata(0, metadata.writeRef(), nullptr);
    SLANG_CHECK_ABORT(metadata != nullptr);
    bool isUsed = false;
    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 0, isUsed);
    SLANG_CHECK(isUsed);
    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 1, isUsed);
    SLANG_CHECK(isUsed);
}

SLANG_UNIT_TEST(globalConstantBufferBindingWithoutExplicitBinding)
{
    // Without an explicit binding the default constant buffer takes slot 0 and `result`
    // follows it at slot 1.
    const char* source = R"(
        RWStructuredBuffer<uint> result;
        uniform uint value;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main(uint3 tid : SV_DispatchThreadID)
        {
            result[0] = value;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    ComPtr<slang::ISession> session;
    auto program =
        _linkProgram(globalSession, SLANG_SPIRV, "spirv_1_5", nullptr, 0, source, session);

    const auto slot = slang::ParameterCategory::DescriptorTableSlot;

    auto globals = program->getLayout()->getGlobalParamsVarLayout();
    auto globalsTypeLayout = globals->getTypeLayout();
    SLANG_CHECK_ABORT(globalsTypeLayout->getKind() == slang::TypeReflection::Kind::ConstantBuffer);
    auto container = globalsTypeLayout->getContainerVarLayout();
    auto element = globalsTypeLayout->getElementVarLayout();
    auto result = element->getTypeLayout()->getFieldByIndex(0);

    SLANG_CHECK(globals->getOffset(slot) == 0);
    SLANG_CHECK(container->getOffset(slot) == 0);
    SLANG_CHECK(element->getOffset(slot) + result->getOffset(slot) == 1);
}

SLANG_UNIT_TEST(globalConstantBufferBindingAfterExplicitD3DRegister)
{
    const char* source = R"(
        struct P { float4 x; }
        ConstantBuffer<P> cb : register(b0);
        RWStructuredBuffer<float4> result : register(u0);
        uniform float4 value;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main(uint3 tid : SV_DispatchThreadID)
        {
            result[0] = cb.x + value;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    ComPtr<slang::ISession> session;
    auto program = _linkProgram(globalSession, SLANG_HLSL, "sm_5_0", nullptr, 0, source, session);

    const auto cbKind = slang::ParameterCategory::ConstantBuffer;

    auto programLayout = program->getLayout();
    auto globals = programLayout->getGlobalParamsVarLayout();
    auto globalsTypeLayout = globals->getTypeLayout();
    SLANG_CHECK_ABORT(globalsTypeLayout->getKind() == slang::TypeReflection::Kind::ConstantBuffer);
    auto container = globalsTypeLayout->getContainerVarLayout();
    auto element = globalsTypeLayout->getElementVarLayout();

    SLANG_CHECK(globals->getOffset(cbKind) == 0);
    SLANG_CHECK(globals->getOffset(cbKind) + container->getOffset(cbKind) == 1);
    SLANG_CHECK(programLayout->getGlobalConstantBufferBinding() == 1);

    auto cb = element->getTypeLayout()->getFieldByIndex(0);
    SLANG_CHECK(
        globals->getOffset(cbKind) + element->getOffset(cbKind) + cb->getOffset(cbKind) == 0);
}

SLANG_UNIT_TEST(globalConstantBufferBindingWithVulkanBindGlobals)
{
    // `-fvk-bind-globals 5 9` places the default constant buffer at set 9, binding 5,
    // while `result` and the texture inside `s` are allocated in set 0.
    const char* source = R"(
        RWStructuredBuffer<float> result;
        struct S { Texture2D t; float f; }
        uniform S s;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main(uint3 tid : SV_DispatchThreadID)
        {
            result[0] = s.f + s.t.Load(int3(0)).x;
        }
    )";

    slang::CompilerOptionEntry bindGlobals = {};
    bindGlobals.name = slang::CompilerOptionName::VulkanBindGlobals;
    bindGlobals.value.kind = slang::CompilerOptionValueKind::Int;
    bindGlobals.value.intValue0 = 5;
    bindGlobals.value.intValue1 = 9;

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    ComPtr<slang::ISession> session;
    auto program =
        _linkProgram(globalSession, SLANG_SPIRV, "spirv_1_5", &bindGlobals, 1, source, session);

    const auto slot = slang::ParameterCategory::DescriptorTableSlot;

    auto programLayout = program->getLayout();
    auto globals = programLayout->getGlobalParamsVarLayout();
    auto globalsTypeLayout = globals->getTypeLayout();
    SLANG_CHECK_ABORT(globalsTypeLayout->getKind() == slang::TypeReflection::Kind::ConstantBuffer);
    auto container = globalsTypeLayout->getContainerVarLayout();

    SLANG_CHECK(globals->getOffset(slot) == 0);
    SLANG_CHECK(globals->getBindingSpace(slot) == 0);
    SLANG_CHECK(container->getOffset(slot) == 5);
    SLANG_CHECK(container->getBindingSpace(slot) == 9);
    SLANG_CHECK(programLayout->getGlobalConstantBufferBinding() == 5);

    // The default constant buffer's descriptor range is in set 9; the ranges for
    // `result` and `s.t` stay in set 0.
    bool foundConstantBuffer = false;
    bool foundTexture = false;
    for (SlangInt i = 0; i < globalsTypeLayout->getDescriptorSetCount(); ++i)
    {
        auto space = globalsTypeLayout->getDescriptorSetSpaceOffset(i);
        for (SlangInt r = 0; r < globalsTypeLayout->getDescriptorSetDescriptorRangeCount(i); ++r)
        {
            auto index = globalsTypeLayout->getDescriptorSetDescriptorRangeIndexOffset(i, r);
            switch (globalsTypeLayout->getDescriptorSetDescriptorRangeType(i, r))
            {
            case slang::BindingType::ConstantBuffer:
                foundConstantBuffer = true;
                SLANG_CHECK(space == 9 && index == 5);
                break;
            case slang::BindingType::Texture:
                foundTexture = true;
                SLANG_CHECK(space == 0 && index == 1);
                break;
            default:
                SLANG_CHECK(space == 0 && index == 0);
                break;
            }
        }
    }
    SLANG_CHECK(foundConstantBuffer);
    SLANG_CHECK(foundTexture);

    ComPtr<slang::IMetadata> metadata;
    program->getTargetMetadata(0, metadata.writeRef(), nullptr);
    SLANG_CHECK_ABORT(metadata != nullptr);
    bool isUsed = false;
    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 9, 5, isUsed);
    SLANG_CHECK(isUsed);
    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 0, 1, isUsed);
    SLANG_CHECK(isUsed);
    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 9, 6, isUsed);
    SLANG_CHECK(!isUsed);
    metadata->isParameterLocationUsed(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, 9, 0, isUsed);
    SLANG_CHECK(!isUsed);
}

// unit-test-target-desc-session-options.cpp
//
// A field of slang::TargetDesc left at its default must not override the value the session's
// compiler options set for it.

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static const char* kSource = R"(
RWStructuredBuffer<float> output;
uniform float a;
uniform float b;
uniform float c;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = a * b + c;
}
)";

// Compile kSource for `targetDesc` with the session options `entries` and return the code.
static String compileWithSessionOptions(
    slang::IGlobalSession* globalSession,
    slang::TargetDesc const& targetDesc,
    slang::CompilerOptionEntry* entries,
    SlangInt entryCount)
{
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = entries;
    sessionDesc.compilerOptionEntryCount = uint32_t(entryCount);

    ComPtr<slang::ISession> session;
    if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
        return String();

    ComPtr<slang::IBlob> diagnostics;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", kSource, diagnostics.writeRef());
    if (!module)
        return String();

    ComPtr<slang::IComponentType> linked;
    module->link(linked.writeRef(), diagnostics.writeRef());
    if (!linked)
        return String();

    ComPtr<slang::IBlob> code;
    linked->getTargetCode(0, code.writeRef(), diagnostics.writeRef());
    if (!code)
        return String();
    return String(
        (const char*)code->getBufferPointer(),
        (const char*)code->getBufferPointer() + code->getBufferSize());
}

// `FloatingPointMode` precise in the session options reaches a target whose TargetDesc leaves
// `floatingPointMode` at its default: on SPIR-V the multiply and the add get NoContraction.
SLANG_UNIT_TEST(targetDescDefaultKeepsSessionFloatingPointMode)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::CompilerOptionEntry precise = {};
    precise.name = slang::CompilerOptionName::FloatingPointMode;
    precise.value.kind = slang::CompilerOptionValueKind::Int;
    precise.value.intValue0 = SLANG_FLOATING_POINT_MODE_PRECISE;

    String withPrecise = compileWithSessionOptions(globalSession, targetDesc, &precise, 1);
    SLANG_CHECK_ABORT(withPrecise.getLength() != 0);
    SLANG_CHECK(withPrecise.indexOf(toSlice("NoContraction")) != -1);

    // Control: without the session option there is no NoContraction.
    String withDefault = compileWithSessionOptions(globalSession, targetDesc, nullptr, 0);
    SLANG_CHECK_ABORT(withDefault.getLength() != 0);
    SLANG_CHECK(withDefault.indexOf(toSlice("NoContraction")) == -1);

    // A mode set in the TargetDesc still takes precedence over the session's.
    targetDesc.floatingPointMode = SLANG_FLOATING_POINT_MODE_FAST;
    String targetFast = compileWithSessionOptions(globalSession, targetDesc, &precise, 1);
    SLANG_CHECK_ABORT(targetFast.getLength() != 0);
    SLANG_CHECK(targetFast.indexOf(toSlice("NoContraction")) == -1);
}

// `LineDirectiveMode` none in the session options reaches a target whose TargetDesc leaves
// `lineDirectiveMode` at its default.
SLANG_UNIT_TEST(targetDescDefaultKeepsSessionLineDirectiveMode)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("cs_6_0");

    slang::CompilerOptionEntry noLines = {};
    noLines.name = slang::CompilerOptionName::LineDirectiveMode;
    noLines.value.kind = slang::CompilerOptionValueKind::Int;
    noLines.value.intValue0 = SLANG_LINE_DIRECTIVE_MODE_NONE;

    String withNone = compileWithSessionOptions(globalSession, targetDesc, &noLines, 1);
    SLANG_CHECK_ABORT(withNone.getLength() != 0);
    SLANG_CHECK(withNone.indexOf(toSlice("#line")) == -1);

    // Control: the default mode writes #line directives.
    String withDefault = compileWithSessionOptions(globalSession, targetDesc, nullptr, 0);
    SLANG_CHECK_ABORT(withDefault.getLength() != 0);
    SLANG_CHECK(withDefault.indexOf(toSlice("#line")) != -1);
}

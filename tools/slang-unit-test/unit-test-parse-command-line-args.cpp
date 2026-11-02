// unit-test-parse-command-line-args.cpp

#include "core/slang-string.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

namespace
{

using namespace Slang;

struct ParseCommandLineArgsTestContext
{
    ParseCommandLineArgsTestContext(UnitTestContext* context)
        : m_unitTestContext(context)
    {
    }

    SlangResult runTests()
    {
        slang::IGlobalSession* slangSession = m_unitTestContext->slangGlobalSession;

        // Test multiple targets with different formats and profiles
        // Note: parseCommandLineArguments expects arguments WITHOUT the program name
        const char* argv[] = {
            "-target",
            "spirv",
            "-o",
            "out1.spv",
            "-profile",
            "sm_5_0",
            "-target",
            "dxil",
            "-o",
            "out2.dxil",
            "-profile",
            "sm_6_0",
            "--",
            "shader.slang"};
        int argc = SLANG_COUNT_OF(argv);

        slang::SessionDesc sessionDesc{};
        ComPtr<ISlangUnknown> allocation;

        SLANG_RETURN_ON_FAIL(slangSession->parseCommandLineArguments(
            argc,
            argv,
            &sessionDesc,
            allocation.writeRef()));

        const auto findCompilerOptionEntry =
            [](const slang::TargetDesc& targetDesc,
               slang::CompilerOptionName optionName) -> const slang::CompilerOptionEntry*
        {
            if (!targetDesc.compilerOptionEntries)
                return nullptr;

            for (SlangInt i = 0; i < targetDesc.compilerOptionEntryCount; ++i)
            {
                const auto& entry = targetDesc.compilerOptionEntries[i];
                if (entry.name == optionName)
                    return &entry;
            }
            return nullptr;
        };

        // Verify we have 2 targets
        SLANG_CHECK(sessionDesc.targetCount == 2);
        if (sessionDesc.targetCount != 2)
            return SLANG_FAIL;

        // Verify first target is SPIRV with sm_5_0
        const auto& target0 = sessionDesc.targets[0];
        SLANG_CHECK(target0.format == SLANG_SPIRV);
        if (target0.format != SLANG_SPIRV)
            return SLANG_FAIL;

        auto profile_sm_5_0 = slangSession->findProfile("sm_5_0");
        SLANG_CHECK(target0.profile == profile_sm_5_0);
        if (target0.profile != profile_sm_5_0)
            return SLANG_FAIL;

        const auto* targetOption0 =
            findCompilerOptionEntry(target0, slang::CompilerOptionName::Target);
        SLANG_CHECK(targetOption0 != nullptr);
        if (!targetOption0)
            return SLANG_FAIL;
        SLANG_CHECK(targetOption0->value.intValue0 == static_cast<int32_t>(target0.format));
        if (targetOption0->value.intValue0 != static_cast<int32_t>(target0.format))
            return SLANG_FAIL;

        const auto* profileOption0 =
            findCompilerOptionEntry(target0, slang::CompilerOptionName::Profile);
        SLANG_CHECK(profileOption0 != nullptr);
        if (!profileOption0)
            return SLANG_FAIL;
        SLANG_CHECK(profileOption0->value.intValue0 == static_cast<int32_t>(profile_sm_5_0));
        if (profileOption0->value.intValue0 != static_cast<int32_t>(profile_sm_5_0))
            return SLANG_FAIL;

        // Verify second target is DXIL with sm_6_0
        const auto& target1 = sessionDesc.targets[1];
        SLANG_CHECK(target1.format == SLANG_DXIL);
        if (target1.format != SLANG_DXIL)
            return SLANG_FAIL;

        auto profile_sm_6_0 = slangSession->findProfile("sm_6_0");
        SLANG_CHECK(target1.profile == profile_sm_6_0);
        if (target1.profile != profile_sm_6_0)
            return SLANG_FAIL;

        const auto* targetOption1 =
            findCompilerOptionEntry(target1, slang::CompilerOptionName::Target);
        SLANG_CHECK(targetOption1 != nullptr);
        if (!targetOption1)
            return SLANG_FAIL;
        SLANG_CHECK(targetOption1->value.intValue0 == static_cast<int32_t>(target1.format));
        if (targetOption1->value.intValue0 != static_cast<int32_t>(target1.format))
            return SLANG_FAIL;

        const auto* profileOption1 =
            findCompilerOptionEntry(target1, slang::CompilerOptionName::Profile);
        SLANG_CHECK(profileOption1 != nullptr);
        if (!profileOption1)
            return SLANG_FAIL;
        SLANG_CHECK(profileOption1->value.intValue0 == static_cast<int32_t>(profile_sm_6_0));
        if (profileOption1->value.intValue0 != static_cast<int32_t>(profile_sm_6_0))
            return SLANG_FAIL;

        return SLANG_OK;
    }

    UnitTestContext* m_unitTestContext;
};

// A failed parse preserves the caller's existing session description so it remains usable;
// the auxiliary allocation is cleared because no parsed data is returned.
void checkParseFails(slang::IGlobalSession* globalSession, int argc, const char* const* argv)
{
    // Non-default starting values let the checks below tell an untouched output from one that was
    // written. The allocation sentinel is released only if the call wrongly succeeds, in which case
    // the call has replaced it with a real allocation.
    const slang::TargetDesc callerTarget{};
    const slang::CompilerOptionEntry callerEntry{};
    slang::SessionDesc sessionDesc{};
    sessionDesc.targets = &callerTarget;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = &callerEntry;
    sessionDesc.compilerOptionEntryCount = 1;
    ISlangUnknown* allocation = reinterpret_cast<ISlangUnknown*>(uintptr_t(1));

    const SlangResult result =
        globalSession->parseCommandLineArguments(argc, argv, &sessionDesc, &allocation);
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(allocation == nullptr);
    SLANG_CHECK(sessionDesc.targets == &callerTarget);
    SLANG_CHECK(sessionDesc.targetCount == 1);
    SLANG_CHECK(sessionDesc.compilerOptionEntries == &callerEntry);
    SLANG_CHECK(sessionDesc.compilerOptionEntryCount == 1);

    if (SLANG_SUCCEEDED(result))
        allocation->release();
}

} // namespace

SLANG_UNIT_TEST(parseCommandLineArgs)
{
    ParseCommandLineArgsTestContext context(unitTestContext);

    const auto result = context.runTests();

    SLANG_CHECK(SLANG_SUCCEEDED(result));
}

SLANG_UNIT_TEST(parseCommandLineArgsReportsParseErrors)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* missingTargetName[] = {"-target"};
    checkParseFails(globalSession, SLANG_COUNT_OF(missingTargetName), missingTargetName);

    const char* unknownOption[] = {"-bogus-option"};
    checkParseFails(globalSession, SLANG_COUNT_OF(unknownOption), unknownOption);

    const char* unknownTarget[] = {"-target", "bogus"};
    checkParseFails(globalSession, SLANG_COUNT_OF(unknownTarget), unknownTarget);

    const char* missingProfileName[] = {"-profile"};
    checkParseFails(globalSession, SLANG_COUNT_OF(missingProfileName), missingProfileName);

    // A profile that implies no target is rejected, as slangc rejects it (E00042): the session
    // desc has no target to carry it, and `createSession` takes each target's profile from its
    // `TargetDesc`.
    const char* profileWithoutTarget[] = {"-profile", "spirv_1_6"};
    checkParseFails(globalSession, SLANG_COUNT_OF(profileWithoutTarget), profileWithoutTarget);

    const char* conflictingProfiles[] = {"-profile", "sm_5_0", "-profile", "glsl_450"};
    checkParseFails(globalSession, SLANG_COUNT_OF(conflictingProfiles), conflictingProfiles);

    // These errors are reported after all arguments have been read, at the end of the parse.
    const char* optionBeforeTargets[] =
        {"-floating-point-mode", "precise", "-target", "hlsl", "-target", "spirv"};
    checkParseFails(globalSession, SLANG_COUNT_OF(optionBeforeTargets), optionBeforeTargets);

    const char* duplicateTarget[] = {"-target", "spirv", "-target", "hlsl", "-target", "spirv"};
    checkParseFails(globalSession, SLANG_COUNT_OF(duplicateTarget), duplicateTarget);

    const char* conflictingTargetProfiles[] =
        {"-target", "dxil", "-profile", "sm_6_0", "-profile", "sm_6_5"};
    checkParseFails(
        globalSession,
        SLANG_COUNT_OF(conflictingTargetProfiles),
        conflictingTargetProfiles);

    const char* profileBeforeTargets[] =
        {"-profile", "sm_5_0", "-target", "dxbc", "-profile", "glsl_450", "-target", "spirv"};
    checkParseFails(globalSession, SLANG_COUNT_OF(profileBeforeTargets), profileBeforeTargets);

    // Entry-point options are ignored, but a malformed one is still rejected.
    const char* unknownStage[] = {"-stage", "bogus"};
    checkParseFails(globalSession, SLANG_COUNT_OF(unknownStage), unknownStage);

    const char* missingEntryPointName[] = {"-entry"};
    checkParseFails(globalSession, SLANG_COUNT_OF(missingEntryPointName), missingEntryPointName);

    // Valid argument lists, with and without a target, still succeed, so the failures above come
    // from the invalid arguments rather than from the API itself.
    {
        slang::SessionDesc sessionDesc{};
        ComPtr<ISlangUnknown> allocation;
        SLANG_CHECK(SLANG_SUCCEEDED(
            globalSession
                ->parseCommandLineArguments(0, nullptr, &sessionDesc, allocation.writeRef())));
        SLANG_CHECK(sessionDesc.targetCount == 0);
    }
    {
        const char* argv[] = {"-target", "spirv"};
        slang::SessionDesc sessionDesc{};
        ComPtr<ISlangUnknown> allocation;
        SLANG_CHECK(SLANG_SUCCEEDED(globalSession->parseCommandLineArguments(
            SLANG_COUNT_OF(argv),
            argv,
            &sessionDesc,
            allocation.writeRef())));
        SLANG_CHECK(sessionDesc.targetCount == 1);
        if (sessionDesc.targetCount == 1)
            SLANG_CHECK(sessionDesc.targets[0].format == SLANG_SPIRV);
    }
}

// Return the SPIR-V assembly that a session created from `sessionDesc`, with one SPIR-V target
// added by the caller, generates for a small compute shader. Returns an empty string on failure.
static String compileSpirvAsmWithSession(
    slang::IGlobalSession* globalSession,
    slang::SessionDesc sessionDesc)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    ComPtr<slang::ISession> session;
    if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
        return String();

    const char* source = R"(
        RWStructuredBuffer<float> buffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain()
        {
            buffer[0] = (buffer[0] + 1.25f) * 0.8125f;
        }
    )";
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "fpModeModule",
        "fpModeModule.slang",
        source,
        diagnostics.writeRef()));
    if (!module)
        return String();

    ComPtr<slang::IEntryPoint> entryPoint;
    if (SLANG_FAILED(module->findEntryPointByName("computeMain", entryPoint.writeRef())))
        return String();

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    if (SLANG_FAILED(session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            program.writeRef(),
            diagnostics.writeRef())))
        return String();

    ComPtr<slang::IComponentType> linkedProgram;
    if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagnostics.writeRef())))
        return String();

    ComPtr<slang::IBlob> code;
    if (SLANG_FAILED(
            linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())))
        return String();
    return String(UnownedStringSlice((const char*)code->getBufferPointer(), code->getBufferSize()));
}

// A floating-point mode given without `-target` is not an error for this API: the session desc
// carries it as a session option, and a target that the caller adds to the session inherits it.
// The precise mode is observed in the generated code, where SPIR-V arithmetic carries
// `NoContraction` only in that mode.
SLANG_UNIT_TEST(parseCommandLineArgsFloatingPointModeReachesSessionTargets)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    slang::SessionDesc defaultDesc{};
    ComPtr<ISlangUnknown> defaultAllocation;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        globalSession
            ->parseCommandLineArguments(0, nullptr, &defaultDesc, defaultAllocation.writeRef())));

    const char* argv[] = {"-floating-point-mode", "precise"};
    slang::SessionDesc preciseDesc{};
    ComPtr<ISlangUnknown> preciseAllocation;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->parseCommandLineArguments(
        SLANG_COUNT_OF(argv),
        argv,
        &preciseDesc,
        preciseAllocation.writeRef())));
    SLANG_CHECK(preciseDesc.targetCount == 0);

    const String defaultCode = compileSpirvAsmWithSession(globalSession, defaultDesc);
    const String preciseCode = compileSpirvAsmWithSession(globalSession, preciseDesc);
    SLANG_CHECK_ABORT(defaultCode.getLength() != 0);
    SLANG_CHECK_ABORT(preciseCode.getLength() != 0);
    SLANG_CHECK(defaultCode.getUnownedSlice().indexOf(toSlice("NoContraction")) < 0);
    SLANG_CHECK(preciseCode.getUnownedSlice().indexOf(toSlice("NoContraction")) >= 0);
}

// Parse `argv`, which must succeed, and check that it yields `expectedTargetCount` targets, the
// first of format `expectedFormat`.
static void checkParseTargets(
    slang::IGlobalSession* globalSession,
    int argc,
    const char* const* argv,
    SlangInt expectedTargetCount,
    SlangCompileTarget expectedFormat)
{
    slang::SessionDesc sessionDesc{};
    ComPtr<ISlangUnknown> allocation;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        globalSession->parseCommandLineArguments(argc, argv, &sessionDesc, allocation.writeRef())));
    SLANG_CHECK(sessionDesc.targetCount == expectedTargetCount);
    if (sessionDesc.targetCount > 0)
        SLANG_CHECK(sessionDesc.targets[0].format == expectedFormat);
}

// A profile that implies a stage is session configuration. The caller selects entry points
// later, so the stage needs no `-entry` here, and the target keeps the full profile.
SLANG_UNIT_TEST(parseCommandLineArgsAcceptsStageProfileWithoutEntryPoint)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* argv[] = {"-target", "dxil", "-profile", "cs_6_0"};
    slang::SessionDesc sessionDesc{};
    ComPtr<ISlangUnknown> allocation;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->parseCommandLineArguments(
        SLANG_COUNT_OF(argv),
        argv,
        &sessionDesc,
        allocation.writeRef())));
    SLANG_CHECK_ABORT(sessionDesc.targetCount == 1);
    SLANG_CHECK(sessionDesc.targets[0].format == SLANG_DXIL);
    SLANG_CHECK(sessionDesc.targets[0].profile == globalSession->findProfile("cs_6_0"));

    ComPtr<slang::ISession> session;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));
}

// Source files, entry-point options and output paths configure a compilation rather than a
// session, so they are accepted and ignored. An `-o` path still infers a target when no
// `-target` is given.
SLANG_UNIT_TEST(parseCommandLineArgsIgnoresCompileOptions)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* entryPointAndStage[] = {"-entry", "main", "-stage", "compute"};
    checkParseTargets(
        globalSession,
        SLANG_COUNT_OF(entryPointAndStage),
        entryPointAndStage,
        0,
        SLANG_TARGET_UNKNOWN);

    const char* stageOnly[] = {"-stage", "compute"};
    checkParseTargets(globalSession, SLANG_COUNT_OF(stageOnly), stageOnly, 0, SLANG_TARGET_UNKNOWN);

    const char* targetAndStage[] = {"-target", "spirv", "-stage", "compute"};
    checkParseTargets(
        globalSession,
        SLANG_COUNT_OF(targetAndStage),
        targetAndStage,
        1,
        SLANG_SPIRV);

    const char* twoOutputsForOneTarget[] = {"-target", "spirv", "-o", "a.spv", "-o", "b.spv"};
    checkParseTargets(
        globalSession,
        SLANG_COUNT_OF(twoOutputsForOneTarget),
        twoOutputsForOneTarget,
        1,
        SLANG_SPIRV);

    const char* outputImpliesTarget[] = {"-o", "foo.spv"};
    checkParseTargets(
        globalSession,
        SLANG_COUNT_OF(outputImpliesTarget),
        outputImpliesTarget,
        1,
        SLANG_SPIRV);

    // These files do not exist; the parse neither loads nor checks them.
    const char* sourceFile[] = {"missing-file.slang"};
    checkParseTargets(
        globalSession,
        SLANG_COUNT_OF(sourceFile),
        sourceFile,
        0,
        SLANG_TARGET_UNKNOWN);

    const char* moduleFile[] = {"missing-module.slang-module"};
    checkParseTargets(
        globalSession,
        SLANG_COUNT_OF(moduleFile),
        moduleFile,
        0,
        SLANG_TARGET_UNKNOWN);
}

// A compile request configured through the deprecated API still resolves entry points when its
// command line is parsed, so a stage with no entry point fails there. It keeps a floating-point
// mode given with no `-target` for the targets added to it later.
SLANG_UNIT_TEST(parseCommandLineArgsCompileRequestKeepsCompileChecks)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    SLANG_ALLOW_DEPRECATED_BEGIN
    {
        ComPtr<slang::ICompileRequest> request;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createCompileRequest(request.writeRef())));
        const char* argv[] = {"-target", "dxil", "-profile", "cs_6_0"};
        SLANG_CHECK(SLANG_FAILED(request->processCommandLineArguments(argv, SLANG_COUNT_OF(argv))));
        UnownedStringSlice diagnostics(request->getDiagnosticOutput());
        SLANG_CHECK(diagnostics.indexOf(toSlice("E00033")) >= 0);
    }
    {
        ComPtr<slang::ICompileRequest> request;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createCompileRequest(request.writeRef())));
        const char* argv[] = {"-floating-point-mode", "precise"};
        SLANG_CHECK(
            SLANG_SUCCEEDED(request->processCommandLineArguments(argv, SLANG_COUNT_OF(argv))));
    }
    SLANG_ALLOW_DEPRECATED_END
}

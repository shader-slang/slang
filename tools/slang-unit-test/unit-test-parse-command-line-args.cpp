// unit-test-parse-command-line-args.cpp

#include "core/slang-string.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Check that parsing `argv` fails, leaves the caller's session description unchanged so that it
// remains usable, and sets the auxiliary allocation to null because no parsed data is returned.
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

    // A session description rejects every target option given before several targets, since
    // it applies to none of them (E00045).
    const char* wholeProgramBeforeTargets[] =
        {"-whole-program", "-target", "hlsl", "-target", "spirv"};
    checkParseFails(
        globalSession,
        SLANG_COUNT_OF(wholeProgramBeforeTargets),
        wholeProgramBeforeTargets);

    const char* capabilityBeforeTargets[] =
        {"-capability", "spirv_1_5", "-target", "spirv", "-target", "hlsl"};
    checkParseFails(
        globalSession,
        SLANG_COUNT_OF(capabilityBeforeTargets),
        capabilityBeforeTargets);

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

// Parse `argv`, which must succeed, into `outSessionDesc`, whose arrays `outAllocation` keeps
// alive.
static void parseSessionDesc(
    slang::IGlobalSession* globalSession,
    int argc,
    const char* const* argv,
    slang::SessionDesc& outSessionDesc,
    ComPtr<ISlangUnknown>& outAllocation)
{
    outSessionDesc = slang::SessionDesc{};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        globalSession
            ->parseCommandLineArguments(argc, argv, &outSessionDesc, outAllocation.writeRef())));
}

// Return the code that a session created from `sessionDesc` generates for its first target from
// `source`. If `hasEntryPoint` is true, the result is the code for the entry point `computeMain`,
// which `source` must define; otherwise it is the code that `getTargetCode` returns for the
// module. Returns an empty string on failure.
static String generateCodeWithSession(
    slang::IGlobalSession* globalSession,
    const slang::SessionDesc& sessionDesc,
    const char* source,
    bool hasEntryPoint)
{
    ComPtr<slang::ISession> session;
    if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
        return String();

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "sessionCodeModule",
        "sessionCodeModule.slang",
        source,
        diagnostics.writeRef()));
    if (!module)
        return String();

    ComPtr<slang::IComponentType> program;
    if (hasEntryPoint)
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        if (SLANG_FAILED(module->findEntryPointByName("computeMain", entryPoint.writeRef())))
            return String();
        slang::IComponentType* components[] = {module, entryPoint};
        if (SLANG_FAILED(session->createCompositeComponentType(
                components,
                SLANG_COUNT_OF(components),
                program.writeRef(),
                diagnostics.writeRef())))
            return String();
    }
    else
    {
        program = module;
    }

    ComPtr<slang::IComponentType> linkedProgram;
    if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagnostics.writeRef())))
        return String();

    ComPtr<slang::IBlob> code;
    const SlangResult result =
        hasEntryPoint
            ? linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())
            : linkedProgram->getTargetCode(0, code.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result) || !code)
        return String();
    return String(UnownedStringSlice((const char*)code->getBufferPointer(), code->getBufferSize()));
}

static bool hasWholeProgramOption(const slang::TargetDesc& targetDesc)
{
    for (uint32_t i = 0; i < targetDesc.compilerOptionEntryCount; ++i)
    {
        const auto& entry = targetDesc.compilerOptionEntries[i];
        if (entry.name == slang::CompilerOptionName::GenerateWholeProgram &&
            entry.value.intValue0 != 0)
            return true;
    }
    return false;
}

static bool areOptionEntriesEqual(
    const slang::CompilerOptionEntry& a,
    const slang::CompilerOptionEntry& b)
{
    const auto areStringsEqual = [](const char* x, const char* y)
    { return (x == nullptr || y == nullptr) ? x == y : strcmp(x, y) == 0; };
    return a.name == b.name && a.value.kind == b.value.kind &&
           a.value.intValue0 == b.value.intValue0 && a.value.intValue1 == b.value.intValue1 &&
           areStringsEqual(a.value.stringValue0, b.value.stringValue0) &&
           areStringsEqual(a.value.stringValue1, b.value.stringValue1);
}

static const char* kComputeShaderSource = R"(
    RWStructuredBuffer<float> buffer;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain()
    {
        buffer[0] = (buffer[0] + 1.25f) * 0.8125f;
    }
)";

// Return the SPIR-V assembly for a small compute shader from a session created from `sessionDesc`
// with its targets replaced by one SPIR-V target, as an API caller supplies the targets of a
// parsed session desc that has none.
static String compileSpirvAsmWithSession(
    slang::IGlobalSession* globalSession,
    slang::SessionDesc sessionDesc)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    return generateCodeWithSession(globalSession, sessionDesc, kComputeShaderSource, true);
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

// Check that `argv` parses into a session description with `expectedTargetCount` targets, the
// first of which has the format `expectedFormat`. Callers that expect no target pass
// `SLANG_TARGET_UNKNOWN`, which is then not checked.
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
// later, so the stage needs no `-entry` here. The target keeps only the profile's version.
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
    SLANG_CHECK(sessionDesc.targets[0].profile == globalSession->findProfile("sm_6_0"));

    ComPtr<slang::ISession> session;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));
}

// A stage profile does not restrict the session's target to that stage: with a pixel-stage
// profile, a module with vertex and compute entry points loads, and its compute entry point
// compiles for the compute stage.
SLANG_UNIT_TEST(parseCommandLineArgsStageProfileKeepsOtherStagesUsable)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* source = R"(
        [shader("vertex")]
        float4 vertexMain(float4 position : POSITION) : SV_Position { return position; }

        RWStructuredBuffer<float> outputBuffer;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 id : SV_DispatchThreadID) { outputBuffer[id.x] = 1.0f; }
    )";

    const char* argv[] = {"-target", "hlsl", "-profile", "ps_5_0"};
    slang::SessionDesc sessionDesc;
    ComPtr<ISlangUnknown> allocation;
    parseSessionDesc(globalSession, SLANG_COUNT_OF(argv), argv, sessionDesc, allocation);
    SLANG_CHECK_ABORT(sessionDesc.targetCount == 1);
    SLANG_CHECK(sessionDesc.targets[0].profile == globalSession->findProfile("sm_5_0"));

    const String code = generateCodeWithSession(globalSession, sessionDesc, source, true);
    SLANG_CHECK(code.getUnownedSlice().indexOf(toSlice("[numthreads(1, 1, 1)]")) >= 0);
}

// A session target's version-only profile keeps the explicit capabilities, from `-capability` or
// from the `+` atoms of a `-profile`, as a `TargetDesc` that sets them directly does. The
// `spirv_1_6` capability is observable in the SPIR-V version that the generated code declares.
SLANG_UNIT_TEST(parseCommandLineArgsCapabilitiesMatchDirectConfiguration)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const auto generateWithArgs = [&](int argc, const char* const* argv)
    {
        slang::SessionDesc sessionDesc;
        ComPtr<ISlangUnknown> allocation;
        parseSessionDesc(globalSession, argc, argv, sessionDesc, allocation);
        SLANG_CHECK(sessionDesc.targetCount == 1);
        if (sessionDesc.targetCount == 1)
            SLANG_CHECK(sessionDesc.targets[0].profile == globalSession->findProfile("glsl_450"));
        return generateCodeWithSession(globalSession, sessionDesc, kComputeShaderSource, true);
    };
    const auto generateWithTargetDesc = [&](bool addCapability)
    {
        slang::CompilerOptionEntry capabilityEntry = {};
        capabilityEntry.name = slang::CompilerOptionName::Capability;
        capabilityEntry.value.kind = slang::CompilerOptionValueKind::Int;
        capabilityEntry.value.intValue0 = globalSession->findCapability("spirv_1_6");

        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV_ASM;
        targetDesc.profile = globalSession->findProfile("glsl_450");
        if (addCapability)
        {
            targetDesc.compilerOptionEntries = &capabilityEntry;
            targetDesc.compilerOptionEntryCount = 1;
        }
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        return generateCodeWithSession(globalSession, sessionDesc, kComputeShaderSource, true);
    };

    const char* profileAtomArgv[] = {"-target", "spirv-asm", "-profile", "glsl_450+spirv_1_6"};
    const char* capabilityArgv[] =
        {"-target", "spirv-asm", "-profile", "glsl_450", "-capability", "spirv_1_6"};
    const char* controlArgv[] = {"-target", "spirv-asm", "-profile", "glsl_450"};
    const String profileAtomCode =
        generateWithArgs(SLANG_COUNT_OF(profileAtomArgv), profileAtomArgv);
    const String capabilityCode = generateWithArgs(SLANG_COUNT_OF(capabilityArgv), capabilityArgv);
    const String controlCode = generateWithArgs(SLANG_COUNT_OF(controlArgv), controlArgv);
    const String directCapabilityCode = generateWithTargetDesc(true);
    const String directControlCode = generateWithTargetDesc(false);

    SLANG_CHECK_ABORT(directCapabilityCode.getLength() != 0);
    SLANG_CHECK_ABORT(directControlCode.getLength() != 0);
    SLANG_CHECK(directCapabilityCode.getUnownedSlice().indexOf(toSlice("Version: 1.6")) >= 0);
    SLANG_CHECK(directControlCode.getUnownedSlice().indexOf(toSlice("Version: 1.6")) < 0);
    SLANG_CHECK(profileAtomCode == directCapabilityCode);
    SLANG_CHECK(capabilityCode == directCapabilityCode);
    SLANG_CHECK(controlCode == directControlCode);
}

// Source and module files, entry-point options and output paths configure a compilation rather
// than a session, so they are accepted and ignored. An `-o` path still infers a target when no
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

// Output paths only infer a target when no `-target` is given, and are not retained. An explicit
// `-target hlsl` with a `.spv` output gives a session whose only target generates HLSL.
SLANG_UNIT_TEST(parseCommandLineArgsOutputPathOnlyInfersTarget)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* argv[] = {"-target", "hlsl", "-o", "foo.spv"};
    slang::SessionDesc sessionDesc;
    ComPtr<ISlangUnknown> allocation;
    parseSessionDesc(globalSession, SLANG_COUNT_OF(argv), argv, sessionDesc, allocation);
    SLANG_CHECK_ABORT(sessionDesc.targetCount == 1);
    SLANG_CHECK(sessionDesc.targets[0].format == SLANG_HLSL);
    SLANG_CHECK(!hasWholeProgramOption(sessionDesc.targets[0]));

    const String code =
        generateCodeWithSession(globalSession, sessionDesc, kComputeShaderSource, true);
    SLANG_CHECK(code.getUnownedSlice().indexOf(toSlice("[numthreads(1, 1, 1)]")) >= 0);
}

// `-o x.spv` and `-target spirv` give the same target configuration, and neither selects
// whole-program code generation; only an explicit `-whole-program` does.
SLANG_UNIT_TEST(parseCommandLineArgsOutputPathDoesNotSelectWholeProgram)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* outputArgv[] = {"-o", "x.spv"};
    slang::SessionDesc outputDesc;
    ComPtr<ISlangUnknown> outputAllocation;
    parseSessionDesc(
        globalSession,
        SLANG_COUNT_OF(outputArgv),
        outputArgv,
        outputDesc,
        outputAllocation);

    const char* targetArgv[] = {"-target", "spirv"};
    slang::SessionDesc targetDesc;
    ComPtr<ISlangUnknown> targetAllocation;
    parseSessionDesc(
        globalSession,
        SLANG_COUNT_OF(targetArgv),
        targetArgv,
        targetDesc,
        targetAllocation);

    SLANG_CHECK_ABORT(outputDesc.targetCount == 1 && targetDesc.targetCount == 1);
    const slang::TargetDesc& fromOutput = outputDesc.targets[0];
    const slang::TargetDesc& fromTarget = targetDesc.targets[0];
    SLANG_CHECK(fromOutput.format == SLANG_SPIRV && fromTarget.format == SLANG_SPIRV);
    SLANG_CHECK(fromOutput.profile == fromTarget.profile);
    SLANG_CHECK(fromOutput.flags == fromTarget.flags);
    // Both parses declare the target through the same code with the same options, and an option
    // set serializes its entries in insertion order, so the entries can be compared by index.
    SLANG_CHECK_ABORT(fromOutput.compilerOptionEntryCount == fromTarget.compilerOptionEntryCount);
    for (uint32_t i = 0; i < fromOutput.compilerOptionEntryCount; ++i)
    {
        SLANG_CHECK(areOptionEntriesEqual(
            fromOutput.compilerOptionEntries[i],
            fromTarget.compilerOptionEntries[i]));
    }
    SLANG_CHECK(!hasWholeProgramOption(fromOutput));
    SLANG_CHECK(!hasWholeProgramOption(fromTarget));

    const String outputCode =
        generateCodeWithSession(globalSession, outputDesc, kComputeShaderSource, true);
    const String targetCode =
        generateCodeWithSession(globalSession, targetDesc, kComputeShaderSource, true);
    SLANG_CHECK(outputCode.getLength() != 0);
    SLANG_CHECK(outputCode == targetCode);

    const char* wholeProgramArgv[] = {"-target", "spirv", "-whole-program"};
    slang::SessionDesc wholeProgramDesc;
    ComPtr<ISlangUnknown> wholeProgramAllocation;
    parseSessionDesc(
        globalSession,
        SLANG_COUNT_OF(wholeProgramArgv),
        wholeProgramArgv,
        wholeProgramDesc,
        wholeProgramAllocation);
    SLANG_CHECK_ABORT(wholeProgramDesc.targetCount == 1);
    SLANG_CHECK(hasWholeProgramOption(wholeProgramDesc.targets[0]));
}

// An explicit `-whole-program` reaches code generation: an HLSL library target emits an exported
// function with `export` only in whole-program mode.
SLANG_UNIT_TEST(parseCommandLineArgsWholeProgramReachesCodeGeneration)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* source = R"(
        export float scaleValue(float x)
        {
            return (x + 1.25f) * 0.8125f;
        }
    )";

    const char* libraryArgv[] = {"-target", "hlsl", "-profile", "lib_6_6"};
    slang::SessionDesc libraryDesc;
    ComPtr<ISlangUnknown> libraryAllocation;
    parseSessionDesc(
        globalSession,
        SLANG_COUNT_OF(libraryArgv),
        libraryArgv,
        libraryDesc,
        libraryAllocation);

    const char* wholeProgramArgv[] = {"-target", "hlsl", "-profile", "lib_6_6", "-whole-program"};
    slang::SessionDesc wholeProgramDesc;
    ComPtr<ISlangUnknown> wholeProgramAllocation;
    parseSessionDesc(
        globalSession,
        SLANG_COUNT_OF(wholeProgramArgv),
        wholeProgramArgv,
        wholeProgramDesc,
        wholeProgramAllocation);

    const String libraryCode = generateCodeWithSession(globalSession, libraryDesc, source, false);
    const String wholeProgramCode =
        generateCodeWithSession(globalSession, wholeProgramDesc, source, false);
    SLANG_CHECK_ABORT(libraryCode.getLength() != 0);
    SLANG_CHECK_ABORT(wholeProgramCode.getLength() != 0);
    SLANG_CHECK(libraryCode.getUnownedSlice().indexOf(toSlice("export")) < 0);
    SLANG_CHECK(wholeProgramCode.getUnownedSlice().indexOf(toSlice("export")) >= 0);
}

SLANG_ALLOW_DEPRECATED_BEGIN

// Return the SPIR-V assembly for `kComputeShaderSource` from a compile request that parses `argv`
// through the deprecated API and then has a SPIR-V target, the source and its entry point added to
// it, as a caller of that API adds them after the parse. Returns an empty string on failure.
static String compileSpirvAsmWithCompileRequest(
    slang::IGlobalSession* globalSession,
    int argc,
    const char* const* argv)
{
    ComPtr<slang::ICompileRequest> request;
    if (SLANG_FAILED(globalSession->createCompileRequest(request.writeRef())) ||
        SLANG_FAILED(request->processCommandLineArguments(argv, argc)))
        return String();

    const int targetIndex = request->addCodeGenTarget(SLANG_SPIRV_ASM);
    request->setTargetProfile(targetIndex, globalSession->findProfile("spirv_1_5"));
    const int translationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "compileRequestModule");
    request->addTranslationUnitSourceString(
        translationUnit,
        "compileRequestModule.slang",
        kComputeShaderSource);
    const int entryPoint =
        request->addEntryPoint(translationUnit, "computeMain", SLANG_STAGE_COMPUTE);
    if (SLANG_FAILED(request->compile()))
        return String();

    ComPtr<ISlangBlob> code;
    if (SLANG_FAILED(request->getEntryPointCodeBlob(entryPoint, targetIndex, code.writeRef())) ||
        !code)
        return String();
    return String(UnownedStringSlice((const char*)code->getBufferPointer(), code->getBufferSize()));
}

// A compile request configured through the deprecated API still resolves entry points when its
// command line is parsed, so a stage with no entry point fails there. It keeps a floating-point
// mode given with no `-target` for the targets added to it later, as the generated SPIR-V shows.
SLANG_UNIT_TEST(parseCommandLineArgsCompileRequestKeepsCompileChecks)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    {
        ComPtr<slang::ICompileRequest> request;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createCompileRequest(request.writeRef())));
        const char* argv[] = {"-target", "dxil", "-profile", "cs_6_0"};
        SLANG_CHECK(SLANG_FAILED(request->processCommandLineArguments(argv, SLANG_COUNT_OF(argv))));
        UnownedStringSlice diagnostics(request->getDiagnosticOutput());
        SLANG_CHECK(diagnostics.indexOf(toSlice("E00033")) >= 0);
    }
    {
        // A compile reports only the floating-point mode among the target options given before
        // several targets, unlike a session description (see
        // `parseCommandLineArgsReportsParseErrors`), for compatibility with existing slangc
        // command lines.
        ComPtr<slang::ICompileRequest> request;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createCompileRequest(request.writeRef())));
        const char* argv[] = {"-whole-program", "-target", "hlsl", "-target", "spirv"};
        SLANG_CHECK(
            SLANG_SUCCEEDED(request->processCommandLineArguments(argv, SLANG_COUNT_OF(argv))));
    }

    const String defaultCode = compileSpirvAsmWithCompileRequest(globalSession, 0, nullptr);
    const char* preciseArgv[] = {"-floating-point-mode", "precise"};
    const String preciseCode =
        compileSpirvAsmWithCompileRequest(globalSession, SLANG_COUNT_OF(preciseArgv), preciseArgv);
    SLANG_CHECK_ABORT(defaultCode.getLength() != 0);
    SLANG_CHECK_ABORT(preciseCode.getLength() != 0);
    SLANG_CHECK(defaultCode.getUnownedSlice().indexOf(toSlice("NoContraction")) < 0);
    SLANG_CHECK(preciseCode.getUnownedSlice().indexOf(toSlice("NoContraction")) >= 0);
}

SLANG_ALLOW_DEPRECATED_END

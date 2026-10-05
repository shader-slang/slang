// unit-test-downstream-args-session.cpp
//
// Tests for how session-level `DownstreamArgs` appear outside code generation: in the session
// description `parseCommandLineArguments` produces, and in the freshness check of a serialized
// module.

#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

static Index countDownstreamArgsEntries(const slang::CompilerOptionEntry* entries, SlangInt count)
{
    Index result = 0;
    for (SlangInt i = 0; i < count; i++)
    {
        if (entries[i].name == slang::CompilerOptionName::DownstreamArgs &&
            entries[i].value.stringValue1 && entries[i].value.stringValue1[0])
            result++;
    }
    return result;
}

// A `-X` argument given on the command line belongs to the session, so the parsed description
// lists it once, in the session entries, and not again in the target entries; passing both to
// `createSession` therefore gives the tool the argument once.
SLANG_UNIT_TEST(parseCommandLineArgumentsKeepsDownstreamArgsInSession)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* argv[] = {"-target", "ptx", "-Xnvrtc", "--fmad=false"};
    slang::SessionDesc sessionDesc = {};
    ComPtr<ISlangUnknown> allocation;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->parseCommandLineArguments(
        SLANG_COUNT_OF(argv),
        argv,
        &sessionDesc,
        allocation.writeRef())));

    SLANG_CHECK(
        countDownstreamArgsEntries(
            sessionDesc.compilerOptionEntries,
            sessionDesc.compilerOptionEntryCount) == 1);
    SLANG_CHECK_ABORT(sessionDesc.targetCount == 1);
    SLANG_CHECK(
        countDownstreamArgsEntries(
            sessionDesc.targets[0].compilerOptionEntries,
            sessionDesc.targets[0].compilerOptionEntryCount) == 0);
}

static SlangResult createSessionWithDownstreamArg(
    slang::IGlobalSession* globalSession,
    ISlangMutableFileSystem* fileSystem,
    const char* spirvOptArg,
    ComPtr<slang::ISession>& outSession)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::CompilerOptionEntry entries[2] = {};
    entries[0].name = slang::CompilerOptionName::DownstreamArgs;
    entries[0].value.kind = slang::CompilerOptionValueKind::String;
    entries[0].value.stringValue0 = "spirv-opt";
    entries[0].value.stringValue1 = spirvOptArg;
    entries[1].name = slang::CompilerOptionName::UseUpToDateBinaryModule;
    entries[1].value.kind = slang::CompilerOptionValueKind::Int;
    entries[1].value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = entries;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(entries);
    sessionDesc.fileSystem = fileSystem;
    return globalSession->createSession(sessionDesc, outSession.writeRef());
}

// A module's own option set does not hold the session's `DownstreamArgs`, but its digest still
// covers them, matching the session-side freshness check: a precompiled module embeds downstream
// output, so a serialized module is up to date only for the same session `DownstreamArgs`.
SLANG_UNIT_TEST(serializedModuleFreshnessCoversSessionDownstreamArgs)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    const char* source = R"(
        module m;
        public int addOne(int x) { return x + 1; }
    )";
    ComPtr<ISlangMutableFileSystem> fileSystem =
        ComPtr<ISlangMutableFileSystem>(new MemoryFileSystem());
    fileSystem->saveFile("m.slang", source, strlen(source));

    ComPtr<ISlangBlob> moduleBlob;
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            createSessionWithDownstreamArg(globalSession, fileSystem, "-O", session)));
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> module;
        module = session->loadModule("m", diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->serialize(moduleBlob.writeRef())));
    }

    {
        ComPtr<slang::ISession> sameArgs;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            createSessionWithDownstreamArg(globalSession, fileSystem, "-O", sameArgs)));
        SLANG_CHECK(sameArgs->isBinaryModuleUpToDate("m.slang", moduleBlob));
    }

    {
        ComPtr<slang::ISession> otherArgs;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            createSessionWithDownstreamArg(globalSession, fileSystem, "-Os", otherArgs)));
        SLANG_CHECK(!otherArgs->isBinaryModuleUpToDate("m.slang", moduleBlob));
    }
}

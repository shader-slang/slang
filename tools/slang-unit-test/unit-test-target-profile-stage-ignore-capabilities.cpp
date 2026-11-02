// unit-test-target-profile-stage-ignore-capabilities.cpp

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
// A compute entry point. Loading it in a session whose target profile names another stage reports
// W36112 (entry point incompatible with profile) for that target.
const char* kSource = R"(
    RWStructuredBuffer<uint> outputBuffer;
    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain() { outputBuffer[0] = 1; }
    )";

struct TargetSpec
{
    const char* profile;
    bool ignoreCapabilities;
};

// Load `kSource` in a session with HLSL targets that use the given profiles, and return the load
// diagnostics. When `sessionIgnoreCapabilities` is set, IgnoreCapabilities is a session-level
// option; otherwise it is set only on the targets that ask for it.
String loadDiagnostics(
    slang::IGlobalSession* globalSession,
    bool sessionIgnoreCapabilities,
    const TargetSpec* targetSpecs,
    Index targetCount)
{
    slang::CompilerOptionEntry ignoreCapabilities = {};
    ignoreCapabilities.name = slang::CompilerOptionName::IgnoreCapabilities;
    ignoreCapabilities.value.kind = slang::CompilerOptionValueKind::Int;
    ignoreCapabilities.value.intValue0 = 1;

    List<slang::TargetDesc> targetDescs;
    for (Index i = 0; i < targetCount; ++i)
    {
        const TargetSpec& spec = targetSpecs[i];
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HLSL;
        targetDesc.profile = globalSession->findProfile(spec.profile);
        if (spec.ignoreCapabilities)
        {
            targetDesc.compilerOptionEntries = &ignoreCapabilities;
            targetDesc.compilerOptionEntryCount = 1;
        }
        targetDescs.add(targetDesc);
    }

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = targetDescs.getBuffer();
    sessionDesc.targetCount = (SlangInt)targetDescs.getCount();
    if (sessionIgnoreCapabilities)
    {
        sessionDesc.compilerOptionEntries = &ignoreCapabilities;
        sessionDesc.compilerOptionEntryCount = 1;
    }

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    session->loadModuleFromSourceString("m", "m.slang", kSource, diagnostics.writeRef());
    if (!diagnostics || diagnostics->getBufferSize() == 0)
        return String();
    return String(UnownedStringSlice(
        (const char*)diagnostics->getBufferPointer(),
        diagnostics->getBufferSize()));
}
} // namespace

// IgnoreCapabilities set in a TargetDesc's compilerOptionEntries must silence the warning that the
// entry point's stage does not match that target's profile, as the same option does when set for
// the whole session, and must not silence it for the session's other targets.
SLANG_UNIT_TEST(targetProfileStageIgnoreCapabilities)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    const UnownedStringSlice incompatible = toSlice("36112");
    const TargetSpec pixel = {"ps_5_0", false};
    const TargetSpec pixelIgnoring = {"ps_5_0", true};

    // No option: the pixel profile target reports the mismatch.
    SLANG_CHECK(loadDiagnostics(globalSession, false, &pixel, 1).indexOf(incompatible) != -1);

    // Session-level option: no warning.
    SLANG_CHECK(loadDiagnostics(globalSession, true, &pixel, 1).indexOf(incompatible) == -1);

    // Target-level option: no warning.
    SLANG_CHECK(
        loadDiagnostics(globalSession, false, &pixelIgnoring, 1).indexOf(incompatible) == -1);

    // Target-level option on one target only: the other target still reports the mismatch, and
    // only that target's profile is named.
    const TargetSpec mixed[] = {pixelIgnoring, {"vs_5_0", false}};
    String diagnostics = loadDiagnostics(globalSession, false, mixed, 2);
    SLANG_CHECK(diagnostics.indexOf(incompatible) != -1);
    SLANG_CHECK(diagnostics.indexOf(toSlice("'vs_5_0'")) != -1);
    SLANG_CHECK(diagnostics.indexOf(toSlice("'ps_5_0'")) == -1);
}

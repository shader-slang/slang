// unit-test-target-ignore-capabilities.cpp

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
// The entry point calls a SPIR-V-only function, so it is not available on HLSL or WGSL, and loading
// the module reports E36107 (entry point uses unavailable capability) for those targets.
const char* kSource = R"(
    [require(spirv)] uint spirvOnly() { return 1; }
    RWStructuredBuffer<uint> outputBuffer;
    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain() { outputBuffer[0] = spirvOnly(); }
    )";

struct TargetSpec
{
    SlangCompileTarget format;
    bool ignoreCapabilities;
};

// Load `kSource` in a session with the given targets and return the load diagnostics. When
// `sessionIgnoreCapabilities` is set, IgnoreCapabilities is a session-level option; otherwise it is
// set only on the targets that ask for it.
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
        targetDesc.format = spec.format;
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

// IgnoreCapabilities set in a TargetDesc's compilerOptionEntries must silence the capability
// errors reported for that target, as the same option does when set for the whole session, and
// must not silence them for the session's other targets.
SLANG_UNIT_TEST(targetIgnoreCapabilities)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    const UnownedStringSlice unavailable = toSlice("E36107");
    // The notes that follow E36107 (for example "see using of 'spirvOnly'") name the function that
    // needs the capability; the error itself does not.
    const UnownedStringSlice provenance = toSlice("spirvOnly");
    const TargetSpec hlsl = {SLANG_HLSL, false};
    const TargetSpec hlslIgnoring = {SLANG_HLSL, true};

    // No option: the HLSL target reports the unavailable capability and its provenance notes.
    String none = loadDiagnostics(globalSession, false, &hlsl, 1);
    SLANG_CHECK(none.indexOf(unavailable) != -1);
    SLANG_CHECK(none.indexOf(provenance) != -1);

    // Session-level option: no error and no notes.
    String session = loadDiagnostics(globalSession, true, &hlsl, 1);
    SLANG_CHECK(session.indexOf(unavailable) == -1);
    SLANG_CHECK(session.indexOf(provenance) == -1);

    // Target-level option: no error and no notes.
    String target = loadDiagnostics(globalSession, false, &hlslIgnoring, 1);
    SLANG_CHECK(target.indexOf(unavailable) == -1);
    SLANG_CHECK(target.indexOf(provenance) == -1);

    // Target-level option on one target only: the other target still reports the error, and only
    // that target is named.
    const TargetSpec mixed[] = {hlslIgnoring, {SLANG_WGSL, false}};
    String diagnostics = loadDiagnostics(globalSession, false, mixed, 2);
    SLANG_CHECK(diagnostics.indexOf(unavailable) != -1);
    SLANG_CHECK(diagnostics.indexOf(toSlice("'wgsl'")) != -1);
    SLANG_CHECK(diagnostics.indexOf(toSlice("'hlsl'")) == -1);
}

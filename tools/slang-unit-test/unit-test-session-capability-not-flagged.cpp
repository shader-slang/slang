// unit-test-session-capability-not-flagged.cpp
//
// Regression test for E36121 false-positive when a capability is set at the
// session level (SessionDesc.compilerOptionEntries) rather than the target
// level.  Multi-backend frameworks (e.g. SGL/SlangPy) legitimately add
// hlsl_nvapi at the session level so D3D12 targets can use it; Vulkan/SPIRV
// targets in the same session should not trigger E36121.

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Build a session with hlsl_nvapi in session-level compiler options and a
// SPIRV target.  Compilation should succeed with no E36121 diagnostic.
SLANG_UNIT_TEST(sessionLevelCapabilityNotFlagged)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // SPIRV target — hlsl_nvapi is not valid here.
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_3");

    // Add hlsl_nvapi at the SESSION level, mimicking what SGL does.
    slang::CompilerOptionEntry capabilityEntry = {};
    capabilityEntry.name = slang::CompilerOptionName::Capability;
    capabilityEntry.value.kind = slang::CompilerOptionValueKind::Int;
    capabilityEntry.value.intValue0 = (int)globalSession->findCapability("hlsl_nvapi");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = &capabilityEntry;
    sessionDesc.compilerOptionEntryCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // Compile a minimal compute shader.  The important thing is no E36121.
    const char* source = R"(
[numthreads(1,1,1)]
void main() {}
)";

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "test",
        "test.slang",
        source,
        diagnosticBlob.writeRef());

    // Extract diagnostics (if any) to a string for inspection.
    String diagnostics;
    if (diagnosticBlob)
        diagnostics = String((const char*)diagnosticBlob->getBufferPointer());

    // Must not contain E36121.
    SLANG_CHECK_MSG(
        diagnostics.indexOf("E36121") == -1,
        "session-level hlsl_nvapi capability incorrectly triggered E36121 on SPIRV target");

    // Module must have been created successfully.
    SLANG_CHECK_MSG(module != nullptr, "compilation failed unexpectedly");
}

// Verify that a capability set at the TARGET level (not session level) on an
// incompatible target still produces E36121.  This ensures the fix did not
// accidentally suppress target-level checks.
SLANG_UNIT_TEST(targetLevelCapabilityStillFlagged)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // Add hlsl_nvapi at the TARGET level (in TargetDesc.compilerOptionEntries).
    slang::CompilerOptionEntry targetCapEntry = {};
    targetCapEntry.name = slang::CompilerOptionName::Capability;
    targetCapEntry.value.kind = slang::CompilerOptionValueKind::Int;
    targetCapEntry.value.intValue0 = (int)globalSession->findCapability("hlsl_nvapi");

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_3");
    targetDesc.compilerOptionEntries = &targetCapEntry;
    targetDesc.compilerOptionEntryCount = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* source = R"(
[numthreads(1,1,1)]
void main() {}
)";

    ComPtr<slang::IBlob> diagnosticBlob;
    session->loadModuleFromSourceString("test", "test.slang", source, diagnosticBlob.writeRef());

    String diagnostics;
    if (diagnosticBlob)
        diagnostics = String((const char*)diagnosticBlob->getBufferPointer());

    SLANG_CHECK_MSG(
        diagnostics.indexOf("E36121") != -1,
        "target-level hlsl_nvapi capability should have triggered E36121 on SPIRV target");
}

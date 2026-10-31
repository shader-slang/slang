// unit-test-wgsl-diagnostic-option.cpp
//
// `CompilerOptionName::WGSLDiagnostic` entries from the session and from a target are merged per
// rule: entries for different rules are all kept, even with the same severity, and for the same
// rule the target's severity wins.

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string>

using namespace Slang;

static slang::CompilerOptionEntry makeWGSLDiagnosticEntry(const char* rule, const char* severity)
{
    slang::CompilerOptionEntry entry = {};
    entry.name = slang::CompilerOptionName::WGSLDiagnostic;
    entry.value.kind = slang::CompilerOptionValueKind::String;
    entry.value.stringValue0 = rule;
    entry.value.stringValue1 = severity;
    return entry;
}

SLANG_UNIT_TEST(wgslDiagnosticOptionsMergePerRule)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    slang::CompilerOptionEntry sessionEntries[] = {
        makeWGSLDiagnosticEntry("subgroup_uniformity", "off"),
        makeWGSLDiagnosticEntry("derivative_uniformity", "warning"),
    };
    slang::CompilerOptionEntry targetEntries[] = {
        makeWGSLDiagnosticEntry("chromium.unreachable_code", "off"),
        makeWGSLDiagnosticEntry("derivative_uniformity", "info"),
    };

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_WGSL;
    targetDesc.compilerOptionEntries = targetEntries;
    targetDesc.compilerOptionEntryCount = SLANG_COUNT_OF(targetEntries);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = sessionEntries;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(sessionEntries);

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    const char* source = R"(
        RWStructuredBuffer<float> outputBuffer;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain()
        {
            outputBuffer[0] = 1.0;
        }
    )";
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module = session->loadModuleFromSourceString(
        "wgsl_diagnostic_option",
        "wgsl-diagnostic-option.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(module->findEntryPointByName("computeMain", entryPoint.writeRef())));

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composite;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        composite.writeRef(),
        diagnostics.writeRef())));
    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(composite->link(linked.writeRef(), diagnostics.writeRef())));

    ComPtr<slang::IBlob> code;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())));
    const std::string wgsl((const char*)code->getBufferPointer(), code->getBufferSize());

    // Both `off` entries are kept, although they have the same severity.
    SLANG_CHECK(wgsl.find("diagnostic(off, subgroup_uniformity);") != std::string::npos);
    SLANG_CHECK(wgsl.find("diagnostic(off, chromium.unreachable_code);") != std::string::npos);
    // For the rule both levels set, the target's severity wins.
    SLANG_CHECK(wgsl.find("diagnostic(info, derivative_uniformity);") != std::string::npos);
    SLANG_CHECK(wgsl.find("diagnostic(warning, derivative_uniformity);") == std::string::npos);
}

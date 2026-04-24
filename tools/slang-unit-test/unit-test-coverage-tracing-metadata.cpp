// unit-test-coverage-tracing-metadata.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Exercises the public `slang::ICoverageTracingMetadata` COM interface
// end-to-end:
//
//   - compile a small compute shader with `-trace-coverage` via the
//     standard compile API (no slangc, no file I/O)
//   - link the program and force codegen so the coverage pass runs
//   - query the per-entry-point metadata
//   - cast to `ICoverageTracingMetadata` via the public `castAs` +
//     `getTypeGuid()` idiom
//   - invoke every virtual method in the vtable and check basic
//     invariants: counter count is non-zero for an instrumented
//     shader, per-entry `(file, line)` strings/values are populated,
//     and buffer binding is reported
//
// This is an ABI-level smoke test: if a future change reorders or
// changes the signature of any virtual method in
// `ICoverageTracingMetadata`, this test will catch it at CI time
// rather than at customer-runtime time.

SLANG_UNIT_TEST(coverageTracingMetadata)
{
    const char* shaderSource = R"(
        RWStructuredBuffer<uint> outputBuffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            uint accum = 0;
            for (uint i = 0; i < 4; ++i)
            {
                if ((i & 1u) == 0u)
                    accum += i;
                else
                    accum += i * 2u;
            }
            outputBuffer[0] = accum;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CPP_SOURCE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry covOption = {};
    covOption.name = slang::CompilerOptionName::TraceCoverage;
    covOption.value.kind = slang::CompilerOptionValueKind::Int;
    covOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = &covOption;
    sessionDesc.compilerOptionEntryCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageTest",
        "coverageTest.slang",
        shaderSource,
        diagnostics.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK(
        session->createCompositeComponentType(components, 2, program.writeRef(), nullptr) ==
        SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    // Force codegen so the coverage pass runs. The compiled blob is
    // not inspected here — metadata is populated as a side effect of
    // the back-end pipeline.
    ComPtr<slang::IBlob> codeBlob;
    SLANG_CHECK(
        linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(
        linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    auto* coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK(coverage != nullptr);

    // The shader has multiple instrumented statements (for loop, if,
    // else, assignments, writeback) — expect at least several slots.
    uint32_t counterCount = coverage->getCounterCount();
    SLANG_CHECK(counterCount > 0);

    // Walk every slot and exercise the per-entry accessors. File
    // string and line number must both be populated for every slot;
    // the synthesizer gives every counter op a real source location.
    for (uint32_t i = 0; i < counterCount; ++i)
    {
        const char* file = coverage->getEntryFile(i);
        uint32_t line = coverage->getEntryLine(i);
        SLANG_CHECK(file != nullptr);
        SLANG_CHECK(line > 0);
    }

    // Out-of-range queries must not crash. No specific return value is
    // part of the public contract here — only that the calls are safe.
    (void)coverage->getEntryFile(counterCount);
    (void)coverage->getEntryLine(counterCount);

    // Buffer binding accessors: for a CPU target, space/binding values
    // may be -1, but both must be callable without crashing.
    (void)coverage->getBufferSpace();
    (void)coverage->getBufferBinding();
}

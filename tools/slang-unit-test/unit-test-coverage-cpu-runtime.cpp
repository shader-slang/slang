// unit-test-coverage-cpu-runtime.cpp

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

using namespace Slang;

// End-to-end runtime test for the CPU shader-coverage path.
//
// The compile-time contract (metadata shape, atomic helper selection,
// GlobalParams packing) is covered by `unit-test-coverage-tracing-metadata.cpp`
// and the filecheck tests under `tests/language-feature/coverage/`. What none
// of those verify is that a host following the documented CPU binding recipe
// actually observes correct execution counts. This test closes that gap:
//
//   1. compile a compute shader with `-trace-coverage` for
//      `SLANG_SHADER_HOST_CALLABLE`,
//   2. discover the hidden `__slang_coverage` buffer through
//      `ISyntheticResourceMetadata`'s CPU uniform-marshaling contract
//      (`uniformOffset` / `uniformStride`),
//   3. write a host-allocated counter buffer view into the global-params
//      payload at that offset,
//   4. invoke the kernel in-process through `getEntryPointHostCallable`,
//   5. validate the counter values against the exact execution counts the
//      dispatch must produce (including a zero count for an unreached
//      branch), for both the default uint64 and opt-down uint32 counter
//      widths.

namespace
{

// The kernel ABI for CPU compute entry points, as declared in
// `prelude/slang-cpp-types.h`: the generated function takes the group-ID
// range to execute, a pointer to entry-point uniform params, and a pointer
// to the global-params payload. Redeclared locally because the prelude
// header is not consumable outside generated code.
struct CpuUInt3
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
};

struct CpuComputeVaryingInput
{
    CpuUInt3 startGroupID;
    CpuUInt3 endGroupID;
};

typedef void (*CpuComputeFunc)(
    CpuComputeVaryingInput* varyingInput,
    void* entryPointParams,
    void* globalParams);

// The CPU representation of a (RW)StructuredBuffer<T> parameter, as declared
// in `prelude/slang-cpp-types.h`: a data pointer followed by an element
// count. This is the value a host must store at `uniformOffset` inside the
// global-params payload to bind the coverage counter buffer.
struct CpuStructuredBufferView
{
    void* data = nullptr;
    size_t count = 0;
};

// Every line we assert on holds exactly one statement, so one coverage
// counter maps to one source line and the expected counts below are exact.
static const char* const kShaderSource = R"(
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = tid.x;
    for (uint i = 0; i < 3; ++i)
    {
        value += i;
    }
    if (tid.x > 100u)
    {
        value += 1000u;
    }
    outputBuffer[tid.x] = value;
}
)";

static const uint32_t kThreadCount = 4;
static const uint32_t kLoopTripCount = 3;

// Return the 1-based line number of the first source line containing
// `needle`, or 0 if not found. Locating asserted lines by content keeps the
// expected-count table valid when the shader source is edited.
static uint32_t findLineContaining(const char* source, const char* needle)
{
    uint32_t line = 1;
    for (const char* cursor = source; *cursor;)
    {
        const char* lineEnd = strchr(cursor, '\n');
        size_t lineLength = lineEnd ? size_t(lineEnd - cursor) : strlen(cursor);
        if (String(UnownedStringSlice(cursor, lineLength)).indexOf(needle) >= 0)
            return line;
        if (!lineEnd)
            break;
        cursor = lineEnd + 1;
        ++line;
    }
    return 0;
}

// Read counter slot `index` from a counter buffer of the given element
// width. The instrumented kernel writes native uint64 or uint32 elements
// depending on `-trace-coverage-counter-width`.
static uint64_t readCounter(const void* counters, int counterByteWidth, uint32_t index)
{
    if (counterByteWidth == 8)
        return ((const uint64_t*)counters)[index];
    return ((const uint32_t*)counters)[index];
}

static void diagnoseIfNeeded(slang::IBlob* diagnostics, const char* label)
{
    if (diagnostics && diagnostics->getBufferSize() > 0)
    {
        fprintf(
            stderr,
            "coverageCpuRuntimeDispatch %s diagnostics:\n%s\n",
            label,
            (const char*)diagnostics->getBufferPointer());
    }
}

// Compile the test shader with coverage tracing at the requested counter
// width, execute one 4-thread group in-process with a host-bound counter
// buffer, and validate both the kernel's output values and the exact
// per-line coverage counts.
static void runCoverageCpuRuntimeTest(slang::IGlobalSession* globalSession, int counterByteWidth)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SHADER_HOST_CALLABLE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry coverageOptions[2] = {};
    coverageOptions[0].name = slang::CompilerOptionName::TraceCoverage;
    coverageOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[0].value.intValue0 = 1;
    coverageOptions[1].name = slang::CompilerOptionName::TraceCoverageCounterByteWidth;
    coverageOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[1].value.intValue0 = counterByteWidth;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = coverageOptions;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(coverageOptions);

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageCpuRuntime",
        "coverageCpuRuntime.slang",
        kShaderSource,
        diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics, "loadModule");
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(components, 2, program.writeRef(), nullptr) ==
        SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    diagnostics.setNull();
    SLANG_CHECK_ABORT(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);
    diagnoseIfNeeded(diagnostics, "link");

    diagnostics.setNull();
    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SlangResult hostCallableResult =
        linked->getEntryPointHostCallable(0, 0, sharedLibrary.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics, "getEntryPointHostCallable");
    SLANG_CHECK_ABORT(hostCallableResult == SLANG_OK);

    auto computeFunc = (CpuComputeFunc)sharedLibrary->findFuncByName("computeMain");
    SLANG_CHECK_ABORT(computeFunc != nullptr);

    // Discover the hidden coverage buffer through the synthetic-resource
    // metadata contract, exactly the way a direct CPU host would.
    diagnostics.setNull();
    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK_ABORT(
        linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    auto coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK_ABORT(coverage != nullptr);
    auto syntheticResources = (slang::ISyntheticResourceMetadata*)metadata->castAs(
        slang::ISyntheticResourceMetadata::getTypeGuid());
    SLANG_CHECK_ABORT(syntheticResources != nullptr);

    const uint32_t counterCount = coverage->getCounterCount();
    SLANG_CHECK_ABORT(counterCount > 0);

    SLANG_CHECK_ABORT(syntheticResources->getResourceCount() == 1);
    slang::SyntheticResourceInfo resourceInfo;
    SLANG_CHECK_ABORT(syntheticResources->getResourceInfo(0, &resourceInfo) == SLANG_OK);
    SLANG_CHECK(resourceInfo.uniformOffset >= 0);
    // The CPU representation of the coverage buffer is a
    // (data pointer, element count) pair; the reported stride is the size of
    // that representation in the global-params payload.
    SLANG_CHECK(resourceInfo.uniformStride == int32_t(sizeof(CpuStructuredBufferView)));

    // The kernel executes in-process, so host and kernel agree on pointer
    // width and the buffer view can be patched in directly.
    uint32_t outputValues[kThreadCount] = {};
    CpuStructuredBufferView outputView;
    outputView.data = outputValues;
    outputView.count = kThreadCount;

    List<uint8_t> counterBytes;
    counterBytes.setCount(Index(counterCount) * counterByteWidth);
    memset(counterBytes.getBuffer(), 0, counterBytes.getCount());
    CpuStructuredBufferView coverageView;
    coverageView.data = counterBytes.getBuffer();
    coverageView.count = counterCount;

    // Build the global-params payload. `outputBuffer` is the only
    // user-declared global, so it sits at offset 0; the synthesized coverage
    // buffer is reported at `uniformOffset`. The two views must not overlap.
    SLANG_CHECK_ABORT(resourceInfo.uniformOffset >= int32_t(sizeof(CpuStructuredBufferView)));
    List<uint8_t> globalParams;
    globalParams.setCount(Index(resourceInfo.uniformOffset) + Index(resourceInfo.uniformStride));
    memset(globalParams.getBuffer(), 0, globalParams.getCount());
    memcpy(globalParams.getBuffer(), &outputView, sizeof(outputView));
    memcpy(
        globalParams.getBuffer() + resourceInfo.uniformOffset,
        &coverageView,
        sizeof(coverageView));

    // Dispatch a single thread group.
    CpuComputeVaryingInput varyingInput;
    varyingInput.endGroupID.x = 1;
    varyingInput.endGroupID.y = 1;
    varyingInput.endGroupID.z = 1;
    computeFunc(&varyingInput, nullptr, globalParams.getBuffer());

    // The instrumented kernel must still compute correct results:
    // outputBuffer[t] = t + (0 + 1 + 2).
    for (uint32_t t = 0; t < kThreadCount; ++t)
        SLANG_CHECK(outputValues[t] == t + 3);

    // Expected exact execution counts per single-statement source line.
    struct ExpectedLine
    {
        const char* statement;
        uint64_t expectedCount;
    };
    const ExpectedLine expectedLines[] = {
        {"uint value = tid.x;", kThreadCount},
        {"value += i;", kThreadCount * kLoopTripCount},
        {"value += 1000u;", 0},
        {"outputBuffer[tid.x] = value;", kThreadCount},
    };

    for (const auto& expected : expectedLines)
    {
        const uint32_t line = findLineContaining(kShaderSource, expected.statement);
        SLANG_CHECK_ABORT(line != 0);

        // Sum every line-entry counter attributed to this source line. The
        // unreached branch must still be instrumented (an entry exists) and
        // read back as zero — that distinguishes "executed zero times" from
        // "not instrumented at all".
        uint32_t entriesOnLine = 0;
        uint64_t totalCount = 0;
        for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
        {
            slang::CoverageEntryInfo entry;
            SLANG_CHECK_ABORT(coverage->getEntryInfo(i, &entry) == SLANG_OK);
            if (entry.kind != slang::CoverageEntryKind::Line)
                continue;
            if (entry.line != line)
                continue;
            SLANG_CHECK_ABORT(entry.counterIndex < counterCount);
            ++entriesOnLine;
            totalCount +=
                readCounter(counterBytes.getBuffer(), counterByteWidth, entry.counterIndex);
        }
        SLANG_CHECK(entriesOnLine == 1);
        SLANG_CHECK(totalCount == expected.expectedCount);
    }
}

} // anonymous namespace

SLANG_UNIT_TEST(coverageCpuRuntimeDispatch)
{
    // Coverage instrumentation is gated off for the LLVM-emitted CPU path
    // (`isCoverageInstrumentationTargetSupported`), and `slang-llvm` availability
    // varies per machine, so this test requires a real downstream C++ compiler
    // and pins it for the host-callable transition on a private global session.
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    const SlangPassThrough cppCompilers[] = {
        SLANG_PASS_THROUGH_VISUAL_STUDIO,
        SLANG_PASS_THROUGH_GCC,
        SLANG_PASS_THROUGH_CLANG,
    };
    SlangPassThrough cppCompiler = SLANG_PASS_THROUGH_NONE;
    for (auto candidate : cppCompilers)
    {
        if (SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(candidate)))
        {
            cppCompiler = candidate;
            break;
        }
    }
    if (cppCompiler == SLANG_PASS_THROUGH_NONE)
    {
        SLANG_IGNORE_TEST;
    }
    globalSession->setDefaultDownstreamCompiler(SLANG_SOURCE_LANGUAGE_CPP, cppCompiler);
    globalSession->setDownstreamCompilerForTransition(
        SLANG_CPP_SOURCE,
        SLANG_SHADER_HOST_CALLABLE,
        cppCompiler);

    // Default 64-bit counters exercise `_slang_atomic_add_u64` at runtime;
    // the opt-down width exercises `_slang_atomic_add_u32`.
    runCoverageCpuRuntimeTest(globalSession, 8);
    runCoverageCpuRuntimeTest(globalSession, 4);
}

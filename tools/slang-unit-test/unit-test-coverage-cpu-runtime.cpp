// unit-test-coverage-cpu-runtime.cpp

#include "core/slang-array-view.h"
#include "core/slang-dictionary.h"
#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <algorithm>
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
//
// A second test uses the same recipe to validate the arm counts of the
// expression-level branch sites (`?:`, `&&`, `||`) under
// `-trace-branch-coverage`.

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

static void diagnoseIfNeeded(slang::IBlob* diagnostics, const char* moduleName, const char* label)
{
    if (diagnostics && diagnostics->getBufferSize() > 0)
    {
        fprintf(
            stderr,
            "%s %s diagnostics:\n%s\n",
            moduleName,
            label,
            (const char*)diagnostics->getBufferPointer());
    }
}

// The state a test inspects after one in-process dispatch of an instrumented
// shader: the coverage metadata produced by the compile, the counter values
// the kernel wrote, and the kernel's own output.
struct CoverageCpuDispatch
{
    ComPtr<slang::IMetadata> metadata;
    slang::ICoverageTracingMetadata* coverage = nullptr;
    uint32_t counterCount = 0;
    int counterByteWidth = 0;
    List<uint8_t> counterBytes;
    uint32_t outputValues[kThreadCount] = {};

    uint64_t getCount(const slang::CoverageEntryInfo& entry) const
    {
        SLANG_CHECK_ABORT(entry.counterIndex < counterCount);
        return readCounter(counterBytes.getBuffer(), counterByteWidth, entry.counterIndex);
    }
};

// Compile `shaderSource` for `SLANG_SHADER_HOST_CALLABLE` with the given
// coverage modes at the requested counter width, bind a host counter buffer
// through the documented CPU marshaling contract, and execute one group of
// `kThreadCount` threads in-process. The shader must declare
// `RWStructuredBuffer<uint> outputBuffer` as its only global and run
// `[numthreads(4, 1, 1)]`.
static void dispatchCoverageShader(
    slang::IGlobalSession* globalSession,
    const char* moduleName,
    const char* shaderSource,
    ConstArrayView<slang::CompilerOptionName> coverageModes,
    int counterByteWidth,
    CoverageCpuDispatch& outDispatch)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SHADER_HOST_CALLABLE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    List<slang::CompilerOptionEntry> coverageOptions;
    auto addIntOption = [&](slang::CompilerOptionName name, int value)
    {
        slang::CompilerOptionEntry option = {};
        option.name = name;
        option.value.kind = slang::CompilerOptionValueKind::Int;
        option.value.intValue0 = value;
        coverageOptions.add(option);
    };
    for (auto mode : coverageModes)
        addIntOption(mode, 1);
    addIntOption(slang::CompilerOptionName::TraceCoverageCounterByteWidth, counterByteWidth);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = coverageOptions.getBuffer();
    sessionDesc.compilerOptionEntryCount = uint32_t(coverageOptions.getCount());

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    String fileName = String(moduleName) + ".slang";
    auto module = session->loadModuleFromSourceString(
        moduleName,
        fileName.getBuffer(),
        shaderSource,
        diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics, moduleName, "loadModule");
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
    diagnoseIfNeeded(diagnostics, moduleName, "link");

    diagnostics.setNull();
    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SlangResult hostCallableResult =
        linked->getEntryPointHostCallable(0, 0, sharedLibrary.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics, moduleName, "getEntryPointHostCallable");
    SLANG_CHECK_ABORT(hostCallableResult == SLANG_OK);

    auto computeFunc = (CpuComputeFunc)sharedLibrary->findFuncByName("computeMain");
    SLANG_CHECK_ABORT(computeFunc != nullptr);

    // Discover the hidden coverage buffer through the synthetic-resource
    // metadata contract, exactly the way a direct CPU host would.
    diagnostics.setNull();
    SLANG_CHECK_ABORT(
        linked->getEntryPointMetadata(
            0,
            0,
            outDispatch.metadata.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    auto coverage = (slang::ICoverageTracingMetadata*)outDispatch.metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK_ABORT(coverage != nullptr);
    auto syntheticResources = (slang::ISyntheticResourceMetadata*)outDispatch.metadata->castAs(
        slang::ISyntheticResourceMetadata::getTypeGuid());
    SLANG_CHECK_ABORT(syntheticResources != nullptr);
    outDispatch.coverage = coverage;

    const uint32_t counterCount = coverage->getCounterCount();
    SLANG_CHECK_ABORT(counterCount > 0);
    outDispatch.counterCount = counterCount;

    SLANG_CHECK_ABORT(syntheticResources->getResourceCount() == 1);
    slang::SyntheticResourceInfo resourceInfo;
    SLANG_CHECK_ABORT(syntheticResources->getResourceInfo(0, &resourceInfo) == SLANG_OK);
    SLANG_CHECK(resourceInfo.uniformOffset >= 0);
    // The CPU representation of the coverage buffer is a
    // (data pointer, element count) pair; the reported stride is the size of
    // that representation in the global-params payload. Abort on mismatch:
    // the payload below is sized from these fields, so continuing with an
    // out-of-contract stride (e.g. the `0` "unavailable" sentinel) would
    // turn a metadata regression into an out-of-bounds write instead of a
    // clean test failure.
    SLANG_CHECK_ABORT(resourceInfo.uniformStride == int32_t(sizeof(CpuStructuredBufferView)));

    // A real host discovers the counter element width from the metadata
    // rather than trusting what it asked for, so exercise that path: read
    // `CoverageBufferInfo::elementByteWidth`, check it matches the requested
    // width, and size the counter storage from the reported value.
    slang::CoverageBufferInfo bufferInfo;
    SLANG_CHECK_ABORT(coverage->getBufferInfo(&bufferInfo) == SLANG_OK);
    SLANG_CHECK_ABORT(bufferInfo.elementByteWidth == uint32_t(counterByteWidth));
    outDispatch.counterByteWidth = int(bufferInfo.elementByteWidth);

    // The kernel executes in-process, so host and kernel agree on pointer
    // width and the buffer view can be patched in directly.
    CpuStructuredBufferView outputView;
    outputView.data = outDispatch.outputValues;
    outputView.count = kThreadCount;

    outDispatch.counterBytes.setCount(Index(counterCount) * outDispatch.counterByteWidth);
    memset(outDispatch.counterBytes.getBuffer(), 0, outDispatch.counterBytes.getCount());
    CpuStructuredBufferView coverageView;
    coverageView.data = outDispatch.counterBytes.getBuffer();
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
}

// Compile the line-coverage test shader at the requested counter width,
// execute one 4-thread group in-process with a host-bound counter buffer,
// and validate both the kernel's output values and the exact per-line
// coverage counts.
static void runCoverageCpuRuntimeTest(slang::IGlobalSession* globalSession, int counterByteWidth)
{
    const slang::CompilerOptionName coverageModes[] = {
        slang::CompilerOptionName::TraceCoverage,
    };
    CoverageCpuDispatch dispatch;
    dispatchCoverageShader(
        globalSession,
        "coverageCpuRuntime",
        kShaderSource,
        makeConstArrayView(coverageModes),
        counterByteWidth,
        dispatch);

    // The instrumented kernel must still compute correct results:
    // outputBuffer[t] = t + (0 + 1 + 2).
    for (uint32_t t = 0; t < kThreadCount; ++t)
        SLANG_CHECK(dispatch.outputValues[t] == t + 3);

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

    auto coverage = dispatch.coverage;
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
            ++entriesOnLine;
            totalCount += dispatch.getCount(entry);
        }
        SLANG_CHECK(entriesOnLine == 1);
        SLANG_CHECK(totalCount == expected.expectedCount);
    }
}

// Expression-level branch sites are attributed to evaluated operands.
// The three sites on the `chained` line are told apart by column.
// The right operands of the `&&` and `||` sites on their own lines are
// calls, so function coverage independently counts how often each right
// operand was evaluated.
static const char* const kExpressionBranchShaderSource = R"(
RWStructuredBuffer<uint> outputBuffer;

bool andRhs(uint t) { return t != 2u; }
bool orRhs(uint t) { return t == 2u; }

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint t = tid.x;
    uint picked = (t == 3u) ? 10u : 20u;
    bool both = (t != 0u) && andRhs(t);
    bool either = (t == 1u) || orRhs(t);
    bool chained = (t >= 1u) && (t == 3u) || (t == 0u);
    bool negated = !((t >= 2u) && (t <= 2u));
    outputBuffer[t] = picked + uint(both) * 100u + uint(either) * 1000u + uint(chained) * 10000u +
                      uint(negated) * 100000u;
}
)";

// Return the 1-based line and column of the first occurrence of `needle` in
// `source`, or line 0 if it does not occur.
static void findSourcePosition(
    const char* source,
    const char* needle,
    uint32_t& outLine,
    uint32_t& outColumn)
{
    outLine = 0;
    outColumn = 0;
    const char* match = strstr(source, needle);
    if (!match)
        return;
    outLine = 1;
    const char* lineStart = source;
    for (const char* cursor = source; cursor < match; ++cursor)
    {
        if (*cursor == '\n')
        {
            ++outLine;
            lineStart = cursor + 1;
        }
    }
    outColumn = uint32_t(match - lineStart) + 1;
}

// Validate the true- and false-arm counts of the one branch site whose
// operator token starts `needle` in the expression-branch shader.
static void checkExpressionBranchSite(
    const CoverageCpuDispatch& dispatch,
    const char* needle,
    uint64_t expectedTrueCount,
    uint64_t expectedFalseCount)
{
    uint32_t line = 0;
    uint32_t column = 0;
    findSourcePosition(kExpressionBranchShaderSource, needle, line, column);
    SLANG_CHECK_ABORT(line != 0);

    uint32_t siteID = 0;
    uint32_t trueArmEntries = 0;
    uint32_t falseArmEntries = 0;
    uint32_t otherArmEntries = 0;
    uint64_t trueCount = 0;
    uint64_t falseCount = 0;
    bool booleanMode = false;
    auto coverage = dispatch.coverage;
    for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK_ABORT(coverage->getEntryInfo(i, &entry) == SLANG_OK);
        if (entry.kind != slang::CoverageEntryKind::Branch)
            continue;
        if (entry.line != line || entry.startColumn != column)
            continue;

        booleanMode = entry.counterMode == slang::CoverageCounterMode::Boolean;
        // Both arms belong to one site.
        if (siteID == 0)
            siteID = entry.branchSiteID;
        SLANG_CHECK(entry.branchSiteID == siteID);

        if (entry.branchArmKind == slang::CoverageBranchArmKind::TrueArm)
        {
            ++trueArmEntries;
            trueCount += dispatch.getCount(entry);
        }
        else if (entry.branchArmKind == slang::CoverageBranchArmKind::FalseArm)
        {
            ++falseArmEntries;
            falseCount += dispatch.getCount(entry);
        }
        else
        {
            ++otherArmEntries;
        }
    }
    SLANG_CHECK(siteID != 0);
    SLANG_CHECK(trueArmEntries == 1);
    SLANG_CHECK(falseArmEntries == 1);
    SLANG_CHECK(otherArmEntries == 0);
    if (booleanMode)
    {
        expectedTrueCount = expectedTrueCount != 0;
        expectedFalseCount = expectedFalseCount != 0;
    }
    SLANG_CHECK(trueCount == expectedTrueCount);
    SLANG_CHECK(falseCount == expectedFalseCount);
}

// Return the summed function-entry count of the function named `name`.
static uint64_t getFunctionEntryCount(const CoverageCpuDispatch& dispatch, const char* name)
{
    uint32_t functionEntries = 0;
    uint64_t count = 0;
    auto coverage = dispatch.coverage;
    for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK_ABORT(coverage->getEntryInfo(i, &entry) == SLANG_OK);
        if (entry.kind != slang::CoverageEntryKind::Function)
            continue;
        if (!entry.functionName ||
            UnownedStringSlice(entry.functionName) != UnownedStringSlice(name))
            continue;
        ++functionEntries;
        count += dispatch.getCount(entry);
    }
    SLANG_CHECK(functionEntries == 1);
    return count;
}

// Execute the expression-branch shader under branch and function coverage
// and validate each site's arm counts against the four threads `t = 0..3`.
static void runCoverageCpuExpressionBranchTest(
    slang::IGlobalSession* globalSession,
    int counterByteWidth,
    bool booleanMode)
{
    List<slang::CompilerOptionName> coverageModes;
    coverageModes.add(slang::CompilerOptionName::TraceFunctionCoverage);
    coverageModes.add(slang::CompilerOptionName::TraceBranchCoverage);
    if (booleanMode)
        coverageModes.add(slang::CompilerOptionName::TraceCoverageBoolean);
    CoverageCpuDispatch dispatch;
    dispatchCoverageShader(
        globalSession,
        "coverageCpuExpressionBranches",
        kExpressionBranchShaderSource,
        coverageModes.getArrayView(),
        counterByteWidth,
        dispatch);

    // The instrumented kernel must still compute correct results.
    const uint32_t expectedOutput[kThreadCount] = {110020u, 101120u, 1020u, 110110u};
    for (uint32_t t = 0; t < kThreadCount; ++t)
        SLANG_CHECK(dispatch.outputValues[t] == expectedOutput[t]);

    // `?:` records its condition: true only for t == 3.
    checkExpressionBranchSite(dispatch, "== 3u) ?", 1, 3);

    // `&&` records its first operand. The true arm evaluates `andRhs`
    // (t = 1, 2, 3) and the false arm short-circuits (t = 0).
    checkExpressionBranchSite(dispatch, "!= 0u) &&", 3, 1);
    SLANG_CHECK(getFunctionEntryCount(dispatch, "andRhs") == (booleanMode ? 1 : 3));

    // `||` records its first operand. The true arm short-circuits (t = 1)
    // and the false arm evaluates `orRhs` (t = 0, 2, 3).
    checkExpressionBranchSite(dispatch, "== 1u) ||", 1, 3);
    SLANG_CHECK(getFunctionEntryCount(dispatch, "orRhs") == (booleanMode ? 1 : 3));

    // Each evaluated operand has its own decision. No extra decision is
    // attributed to the merged result of the inner `&&` expression.
    checkExpressionBranchSite(dispatch, ">= 1u)", 3, 1);
    checkExpressionBranchSite(dispatch, "== 3u) ||", 1, 2);
    checkExpressionBranchSite(dispatch, "== 0u);", 1, 2);

    // A negated short-circuit condition is the merged result of its operands:
    // `t >= 2u` is true for t = 2, 3 and false for t = 0, 1, and `t <= 2u` is
    // evaluated only for t = 2, 3. The negation adds no decision of its own,
    // so exactly two sites sit on that line.
    checkExpressionBranchSite(dispatch, ">= 2u)", 2, 2);
    checkExpressionBranchSite(dispatch, "<= 2u)", 1, 1);
    {
        uint32_t line = 0;
        uint32_t column = 0;
        findSourcePosition(kExpressionBranchShaderSource, "<= 2u)", line, column);
        HashSet<uint32_t> sites;
        for (uint32_t i = 0; i < dispatch.coverage->getEntryCount(); ++i)
        {
            slang::CoverageEntryInfo entry;
            SLANG_CHECK_ABORT(dispatch.coverage->getEntryInfo(i, &entry) == SLANG_OK);
            if (entry.kind == slang::CoverageEntryKind::Branch && entry.line == line)
                sites.add(entry.branchSiteID);
        }
        SLANG_CHECK(sites.getCount() == 2);
    }
}

// Create a private global session whose host-callable transition uses a real
// downstream C++ compiler, or return false when the machine has none.
// Coverage instrumentation is gated off for the LLVM-emitted CPU path
// (`isCoverageInstrumentationTargetSupported`), and `slang-llvm` availability
// varies per machine, so these tests pin the C++ compiler on a session of
// their own.
static bool createCppHostCallableGlobalSession(ComPtr<slang::IGlobalSession>& outGlobalSession)
{
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, outGlobalSession.writeRef()) == SLANG_OK);

    const SlangPassThrough cppCompilers[] = {
        SLANG_PASS_THROUGH_VISUAL_STUDIO,
        SLANG_PASS_THROUGH_GCC,
        SLANG_PASS_THROUGH_CLANG,
    };
    SlangPassThrough cppCompiler = SLANG_PASS_THROUGH_NONE;
    for (auto candidate : cppCompilers)
    {
        if (SLANG_SUCCEEDED(outGlobalSession->checkPassThroughSupport(candidate)))
        {
            cppCompiler = candidate;
            break;
        }
    }
    if (cppCompiler == SLANG_PASS_THROUGH_NONE)
        return false;

    outGlobalSession->setDefaultDownstreamCompiler(SLANG_SOURCE_LANGUAGE_CPP, cppCompiler);
    outGlobalSession->setDownstreamCompilerForTransition(
        SLANG_CPP_SOURCE,
        SLANG_SHADER_HOST_CALLABLE,
        cppCompiler);
    return true;
}

} // anonymous namespace

SLANG_UNIT_TEST(coverageCpuRuntimeDispatch)
{
    ComPtr<slang::IGlobalSession> globalSession;
    if (!createCppHostCallableGlobalSession(globalSession))
    {
        SLANG_IGNORE_TEST;
    }

    // Default 64-bit counters exercise `_slang_atomic_add_u64` at runtime;
    // the opt-down width exercises `_slang_atomic_add_u32`.
    runCoverageCpuRuntimeTest(globalSession, 8);
    runCoverageCpuRuntimeTest(globalSession, 4);
}

SLANG_UNIT_TEST(coverageCpuRuntimeExpressionBranches)
{
    ComPtr<slang::IGlobalSession> globalSession;
    if (!createCppHostCallableGlobalSession(globalSession))
    {
        SLANG_IGNORE_TEST;
    }

    for (int width : {4, 8})
        for (bool booleanMode : {false, true})
            runCoverageCpuExpressionBranchTest(globalSession, width, booleanMode);
}


// These cases exercise region entry, loop re-entry, and early exits. They
// deliberately put several independently executed statements on one line.
SLANG_UNIT_TEST(coverageCpuRuntimeLineRegions)
{
    ComPtr<slang::IGlobalSession> globalSession;
    if (!createCppHostCallableGlobalSession(globalSession))
    {
        SLANG_IGNORE_TEST;
    }
    const char* source = R"(
RWStructuredBuffer<uint> outputBuffer;
uint choose(uint t) { if (t == 0) return 5; return 7; } // early
[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint t = tid.x;
    uint value = 0; if ((t & 1) != 0) value += 1; else value += 2; // sameLine
    for (uint i = 0; i < t; ++i) { if (i == 1) continue; value += i; } // oneLineLoop
    for (uint i = 0; i < 2; ++i) // outer
    {
        for (uint j = 0; j < 3; ++j) // inner
        {
            if (j == 1) break; // breakTest
            value += 10; // innerBody
        }
    }
    bool skipped = t > 100 && t < 200; // skipped
    value += t == 0 ? (t < 2 ? 3 : 4) : ((t == 1 || t == 3) ? 5 : 6); // nested
    uint arm = (t > 1u)
        ? t + 1u // armTrue
        : t + 2u; // armFalse
    uint pick = 0;
    switch (t) // switchLine
    {
    case 0:
        pick = 1;
        break;
    case 1:
        pick = 2;
        break;
    default:
        pick = 3;
        break;
    }
    outputBuffer[t] = value + choose(t) + uint(skipped) + arm + pick;
}
)";
    struct ExpectedLine
    {
        const char* tag;
        uint64_t count;
    };
    const ExpectedLine expectedLines[] = {
        {"// early", 4},
        {"// sameLine", 4},
        {"// oneLineLoop", 10},
        {"// outer", 12},
        {"// inner", 16},
        {"// breakTest", 16},
        {"// innerBody", 8},
        {"// skipped", 4},
        {"// nested", 4},
        {"// armTrue", 2},
        {"// armFalse", 2},
    };
    for (int width : {4, 8})
    {
        for (bool booleanMode : {false, true})
        {
            List<slang::CompilerOptionName> modes;
            modes.add(slang::CompilerOptionName::TraceCoverage);
            modes.add(slang::CompilerOptionName::TraceBranchCoverage);
            modes.add(slang::CompilerOptionName::TraceFunctionCoverage);
            if (booleanMode)
                modes.add(slang::CompilerOptionName::TraceCoverageBoolean);
            CoverageCpuDispatch dispatch;
            dispatchCoverageShader(
                globalSession,
                "coverageCpuLineRegions",
                source,
                modes.getArrayView(),
                width,
                dispatch);
            const uint32_t expectedOutput[] = {33, 38, 41, 42};
            for (uint32_t t = 0; t < kThreadCount; ++t)
                SLANG_CHECK(dispatch.outputValues[t] == expectedOutput[t]);
            for (auto expected : expectedLines)
            {
                auto line = findLineContaining(source, expected.tag);
                // The count of a source line is the maximum over its line entries.
                uint32_t entries = 0;
                uint64_t lineCount = 0;
                for (uint32_t i = 0; i < dispatch.coverage->getEntryCount(); ++i)
                {
                    slang::CoverageEntryInfo entry;
                    SLANG_CHECK_ABORT(dispatch.coverage->getEntryInfo(i, &entry) == SLANG_OK);
                    if (entry.kind != slang::CoverageEntryKind::Line || entry.line != line)
                        continue;
                    ++entries;
                    lineCount = std::max(lineCount, dispatch.getCount(entry));
                }
                SLANG_CHECK(entries >= 1);
                SLANG_CHECK(lineCount == (booleanMode ? 1 : expected.count));
            }
            // Every dispatch arm of a switch, including its default, is attributed to
            // the line of the switch condition, not to its case labels.
            const auto switchLine = findLineContaining(source, "// switchLine");
            uint32_t caseArms = 0;
            uint32_t defaultArms = 0;
            uint64_t caseCount = 0;
            uint64_t defaultCount = 0;
            for (uint32_t i = 0; i < dispatch.coverage->getEntryCount(); ++i)
            {
                slang::CoverageEntryInfo entry;
                SLANG_CHECK_ABORT(dispatch.coverage->getEntryInfo(i, &entry) == SLANG_OK);
                if (entry.kind != slang::CoverageEntryKind::Branch)
                    continue;
                if (entry.branchArmKind == slang::CoverageBranchArmKind::CaseArm)
                {
                    ++caseArms;
                    caseCount += dispatch.getCount(entry);
                    SLANG_CHECK(entry.line == switchLine);
                }
                else if (entry.branchArmKind == slang::CoverageBranchArmKind::DefaultArm)
                {
                    ++defaultArms;
                    defaultCount += dispatch.getCount(entry);
                    SLANG_CHECK(entry.line == switchLine);
                }
            }
            SLANG_CHECK(caseArms == 2);
            SLANG_CHECK(defaultArms == 1);
            SLANG_CHECK(caseCount == 2);
            SLANG_CHECK(defaultCount == (booleanMode ? 1 : 2));
            uint32_t rhsLine, rhsColumn;
            findSourcePosition(source, "< 200", rhsLine, rhsColumn);
            uint32_t rhsEntries = 0;
            for (uint32_t i = 0; i < dispatch.coverage->getEntryCount(); ++i)
            {
                slang::CoverageEntryInfo entry;
                SLANG_CHECK_ABORT(dispatch.coverage->getEntryInfo(i, &entry) == SLANG_OK);
                if (entry.kind == slang::CoverageEntryKind::Branch && entry.line == rhsLine &&
                    entry.startColumn == rhsColumn)
                {
                    ++rhsEntries;
                    SLANG_CHECK(dispatch.getCount(entry) == 0);
                }
            }
            SLANG_CHECK(rhsEntries == 2);
        }
    }
}

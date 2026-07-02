// unit-test-coverage-metal-runtime.cpp

// metal-cpp must be included before any Slang headers: its own headers
// resolve types like `NS::String` by unqualified name internally, and a
// preceding `using namespace Slang` (or Slang core includes) makes those
// lookups ambiguous against `Slang::String`.
#ifdef __APPLE__

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#endif

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

using namespace Slang;

// End-to-end runtime test for the Metal shader-coverage binding path.
//
// The compile-time contract (MetalBuffer resource kind, `space == -1`
// sentinel, metadata shape) is covered by
// `unit-test-coverage-tracing-metadata.cpp` and the filecheck tests under
// `tests/language-feature/coverage/`. What none of those verify is that a
// host following the descriptor-facing Metal binding contract actually
// observes correct execution counts on a GPU. This test closes that gap:
//
//   1. compile a compute shader with `-trace-coverage` (32-bit counters —
//      MSL provides `atomic_fetch_add_explicit` only for 32-bit
//      `atomic_uint`, so 32-bit is the only executable width on Metal) to
//      Metal shading language source,
//   2. discover the hidden `__slang_coverage` buffer's `[[buffer(N)]]`
//      index through `ISyntheticResourceMetadata` (`binding >= 0`,
//      `space == -1`, no CPU/CUDA marshaling fields),
//   3. compile the MSL at runtime with the Metal framework's own compiler
//      (no external toolchain needed), bind a zero-initialized counter
//      buffer at the reported index, and dispatch one thread group,
//   4. read the counters back and validate them against the exact
//      execution counts the dispatch must produce, including a zero count
//      for an unreached branch.
//
// The test runs only on Apple platforms with a Metal device and reports
// `Ignored` everywhere else.

namespace
{

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

static void diagnoseIfNeeded(slang::IBlob* diagnostics, const char* label)
{
    if (diagnostics && diagnostics->getBufferSize() > 0)
    {
        fprintf(
            stderr,
            "coverageMetalRuntimeDispatch %s diagnostics:\n%s\n",
            label,
            (const char*)diagnostics->getBufferPointer());
    }
}

} // anonymous namespace

SLANG_UNIT_TEST(coverageMetalRuntimeDispatch)
{
#if SLANG_APPLE_FAMILY
    // Skip when the machine has no Metal device (e.g. GPU-less CI hosts).
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    if (!device)
    {
        SLANG_IGNORE_TEST;
    }

    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_METAL;
    targetDesc.profile = globalSession->findProfile("metal");

    // MSL only provides 32-bit `atomic_fetch_add_explicit`, so pin the
    // counter width to 4 bytes. Emitting the default 64-bit counters would
    // fail in the Metal compiler with "no matching function for call to
    // 'atomic_fetch_add_explicit'".
    slang::CompilerOptionEntry coverageOptions[2] = {};
    coverageOptions[0].name = slang::CompilerOptionName::TraceCoverage;
    coverageOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[0].value.intValue0 = 1;
    coverageOptions[1].name = slang::CompilerOptionName::TraceCoverageCounterByteWidth;
    coverageOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[1].value.intValue0 = 4;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = coverageOptions;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(coverageOptions);

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageMetalRuntime",
        "coverageMetalRuntime.slang",
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
    ComPtr<slang::IBlob> mslBlob;
    SLANG_CHECK_ABORT(
        linked->getEntryPointCode(0, 0, mslBlob.writeRef(), diagnostics.writeRef()) == SLANG_OK);
    diagnoseIfNeeded(diagnostics, "getEntryPointCode");

    // Discover the hidden coverage buffer through the synthetic-resource
    // metadata contract, exactly the way a direct Metal host would.
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
    // Metal's binding contract: a plain `[[buffer(N)]]` index with no
    // descriptor-space dimension and no CPU/CUDA marshaling location.
    SLANG_CHECK(resourceInfo.binding >= 0);
    SLANG_CHECK(resourceInfo.space == -1);
    SLANG_CHECK(resourceInfo.uniformOffset == -1);
    SLANG_CHECK(resourceInfo.uniformStride == 0);
    // The user's `outputBuffer` is the only reflected global, so it occupies
    // `[[buffer(0)]]` and auto-allocation must have steered the coverage
    // buffer elsewhere.
    SLANG_CHECK_ABORT(resourceInfo.binding != 0);

    // Compile the emitted MSL with the Metal framework's runtime compiler.
    NS::Error* error = nullptr;
    NS::String* mslSource =
        NS::String::string((const char*)mslBlob->getBufferPointer(), NS::UTF8StringEncoding);
    MTL::Library* library = device->newLibrary(mslSource, nullptr, &error);
    if (!library && error)
    {
        fprintf(
            stderr,
            "coverageMetalRuntimeDispatch Metal compile error:\n%s\n",
            error->localizedDescription()->utf8String());
    }
    SLANG_CHECK_ABORT(library != nullptr);

    MTL::Function* function =
        library->newFunction(NS::String::string("computeMain", NS::UTF8StringEncoding));
    SLANG_CHECK_ABORT(function != nullptr);

    error = nullptr;
    MTL::ComputePipelineState* pipeline = device->newComputePipelineState(function, &error);
    if (!pipeline && error)
    {
        fprintf(
            stderr,
            "coverageMetalRuntimeDispatch pipeline error:\n%s\n",
            error->localizedDescription()->utf8String());
    }
    SLANG_CHECK_ABORT(pipeline != nullptr);

    // Shared-storage buffers so the CPU can read results directly back.
    MTL::Buffer* outputBuffer =
        device->newBuffer(kThreadCount * sizeof(uint32_t), MTL::ResourceStorageModeShared);
    MTL::Buffer* counterBuffer =
        device->newBuffer(counterCount * sizeof(uint32_t), MTL::ResourceStorageModeShared);
    SLANG_CHECK_ABORT(outputBuffer != nullptr && counterBuffer != nullptr);
    memset(outputBuffer->contents(), 0, kThreadCount * sizeof(uint32_t));
    memset(counterBuffer->contents(), 0, counterCount * sizeof(uint32_t));

    MTL::CommandQueue* queue = device->newCommandQueue();
    SLANG_CHECK_ABORT(queue != nullptr);
    MTL::CommandBuffer* commandBuffer = queue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();
    encoder->setComputePipelineState(pipeline);
    encoder->setBuffer(outputBuffer, 0, 0);
    encoder->setBuffer(counterBuffer, 0, NS::UInteger(resourceInfo.binding));
    encoder->dispatchThreadgroups(MTL::Size(1, 1, 1), MTL::Size(kThreadCount, 1, 1));
    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
    SLANG_CHECK_ABORT(commandBuffer->status() == MTL::CommandBufferStatusCompleted);

    // The instrumented kernel must still compute correct results:
    // outputBuffer[t] = t + (0 + 1 + 2).
    const uint32_t* outputValues = (const uint32_t*)outputBuffer->contents();
    for (uint32_t t = 0; t < kThreadCount; ++t)
        SLANG_CHECK(outputValues[t] == t + 3);

    // Expected exact execution counts per single-statement source line.
    struct ExpectedLine
    {
        const char* statement;
        uint32_t expectedCount;
    };
    const ExpectedLine expectedLines[] = {
        {"uint value = tid.x;", kThreadCount},
        {"value += i;", kThreadCount * kLoopTripCount},
        {"value += 1000u;", 0},
        {"outputBuffer[tid.x] = value;", kThreadCount},
    };

    const uint32_t* counters = (const uint32_t*)counterBuffer->contents();
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
            totalCount += counters[entry.counterIndex];
        }
        SLANG_CHECK(entriesOnLine == 1);
        SLANG_CHECK(totalCount == expected.expectedCount);
    }

    queue->release();
    counterBuffer->release();
    outputBuffer->release();
    pipeline->release();
    function->release();
    library->release();
    autoreleasePool->release();
    device->release();
#else
    SLANG_IGNORE_TEST;
#endif
}

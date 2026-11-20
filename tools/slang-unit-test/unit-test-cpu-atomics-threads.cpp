// unit-test-cpu-atomics-threads.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <vector>

using namespace Slang;

// The CPU target's atomics are real atomics: a kernel that uses `Interlocked*` and `Atomic<T>`
// is compiled for `SLANG_SHADER_HOST_CALLABLE`, and its group-range function is called from
// several host threads at once, each with its own range of groups. Every result is exact, so a
// lost update (a plain read-modify-write) shows up as a wrong count.

namespace
{

// The kernel ABI for CPU compute entry points (`prelude/slang-cpp-types.h`).
struct CpuUInt3
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
};

// Mirrors `ComputeVaryingInput` in `prelude/slang-cpp-types.h`: the range of groups one call of
// the group-range function runs. `endGroupID` is exclusive on each axis.
struct CpuComputeVaryingInput
{
    CpuUInt3 startGroupID;
    CpuUInt3 endGroupID;
};

typedef void (*CpuComputeFunc)(
    CpuComputeVaryingInput* varyingInput,
    void* entryPointParams,
    void* globalParams);

static const char* const kShaderSource = R"(
[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(
    uint3 tid : SV_DispatchThreadID,
    uniform uint* u,
    uniform int* i,
    uniform uint64_t* u64,
    uniform float* f,
    uniform Atomic<uint>* au,
    uniform Atomic<float>* af,
    uniform Atomic<int64_t>* ai64)
{
    InterlockedAdd(u[0], 1u);
    InterlockedMax(u[1], tid.x);
    InterlockedMin(u[2], tid.x);
    InterlockedOr(u[3], 1u << (tid.x & 31u));
    InterlockedXor(u[4], tid.x);
    uint previous;
    InterlockedAdd(u[5], 1u, previous);
    InterlockedAnd(u[6], ~(1u << (tid.x & 31u)));
    InterlockedAdd(i[0], -1);
    InterlockedAdd(u64[0], 3ull);
    InterlockedAdd(f[0], 1.0f);

    // An increment built from compare-exchange, as a lock-free algorithm does it.
    uint expected;
    InterlockedOr(u[7], 0u, expected);
    for (;;)
    {
        uint original;
        InterlockedCompareExchange(u[7], expected, expected + 1u, original);
        if (original == expected)
            break;
        expected = original;
    }

    au[0].add(2u);
    au[1].max(tid.x);
    au[2].sub(1u);
    uint current = au[3].load();
    while (true)
    {
        uint seen = au[3].compareExchange(current, current + 1u);
        if (seen == current)
            break;
        current = seen;
    }
    au[4].exchange(tid.x);
    af[0].add(0.5f);
    af[1].max(float(tid.x));
    ai64[0].sub(1);
}
)";

// Mirrors the entry-point parameter block of `kShaderSource`: one pointer per `uniform` parameter
// of `computeMain`, in declaration order. An `Atomic<T>` is stored as a plain T, so `au`, `af` and
// `ai64` point at uint32_t, float and int64_t storage.
struct KernelParams
{
    uint32_t* u;
    int32_t* i;
    uint64_t* u64;
    float* f;
    uint32_t* au;
    float* af;
    int64_t* ai64;
};

static void diagnoseIfNeeded(slang::IBlob* diagnostics, const char* label)
{
    if (diagnostics && diagnostics->getBufferSize() > 0)
        fprintf(
            stderr,
            "%s diagnostics:\n%s\n",
            label,
            (const char*)diagnostics->getBufferPointer());
}

} // namespace

SLANG_UNIT_TEST(cpuAtomicsAreThreadSafe)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SHADER_HOST_CALLABLE;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef()) ==
        SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "cpuAtomicsThreads",
        "cpuAtomicsThreads.slang",
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
    SlangResult result =
        linked->getEntryPointHostCallable(0, 0, sharedLibrary.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics, "getEntryPointHostCallable");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));

    auto computeFunc = (CpuComputeFunc)sharedLibrary->findFuncByName("computeMain");
    SLANG_CHECK_ABORT(computeFunc != nullptr);

    uint32_t u[8] = {0, 0, 0xffffffffu, 0, 0, 0, 0xffffffffu, 0};
    int32_t i[1] = {0};
    uint64_t u64[1] = {0};
    float f[1] = {0.0f};
    uint32_t au[5] = {0, 0, 0, 0, 0xffffffffu};
    float af[2] = {0.0f, -1.0f};
    int64_t ai64[1] = {0};
    KernelParams params = {u, i, u64, f, au, af, ai64};

    // 4096 groups of 64 threads, split over the host threads. Every float sum stays below 2^24,
    // so it is exact in any order.
    const uint32_t groupCount = 4096;
    const uint32_t threadsPerGroup = 64;
    const uint32_t total = groupCount * threadsPerGroup;
    uint32_t hostThreadCount = std::thread::hardware_concurrency();
    if (hostThreadCount < 2)
        hostThreadCount = 2;
    if (hostThreadCount > 16)
        hostThreadCount = 16;

    std::vector<std::thread> threads;
    for (uint32_t t = 0; t < hostThreadCount; ++t)
    {
        threads.emplace_back(
            [&, t]()
            {
                CpuComputeVaryingInput range;
                range.startGroupID.x = groupCount * t / hostThreadCount;
                range.endGroupID.x = groupCount * (t + 1) / hostThreadCount;
                range.endGroupID.y = 1;
                range.endGroupID.z = 1;
                computeFunc(&range, &params, nullptr);
            });
    }
    for (auto& thread : threads)
        thread.join();

    SLANG_CHECK(u[0] == total);
    SLANG_CHECK(u[1] == total - 1);
    SLANG_CHECK(u[2] == 0);
    SLANG_CHECK(u[3] == 0xffffffffu);
    SLANG_CHECK(u[4] == 0); // the XOR of 0 .. total - 1, for a total that is a multiple of 4
    SLANG_CHECK(u[5] == total);
    SLANG_CHECK(u[6] == 0);
    SLANG_CHECK(u[7] == total);
    SLANG_CHECK(i[0] == -int32_t(total));
    SLANG_CHECK(u64[0] == 3ull * total);
    SLANG_CHECK(f[0] == float(total));
    SLANG_CHECK(au[0] == 2 * total);
    SLANG_CHECK(au[1] == total - 1);
    SLANG_CHECK(au[2] == uint32_t(0) - total);
    SLANG_CHECK(au[3] == total);
    SLANG_CHECK(au[4] < total);
    SLANG_CHECK(af[0] == 0.5f * float(total));
    SLANG_CHECK(af[1] == float(total - 1));
    SLANG_CHECK(ai64[0] == -int64_t(total));
}

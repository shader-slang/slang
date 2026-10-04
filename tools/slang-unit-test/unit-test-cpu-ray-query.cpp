#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#define SLANG_PRELUDE_NAMESPACE cpu_ray_query_test
#include "slang-cpp-types.h"
#undef SLANG_PRELUDE_NAMESPACE

using namespace Slang;

namespace
{
using namespace cpu_ray_query_test;

// Yields two triangle candidates, then a final procedural candidate. The scripted intersections
// isolate the generated query state machine from geometry traversal and require no RHI or BVH.
struct TestProvider : IRaytracingAccelerationStructure
{
    // Resumes the scripted sequence using the query's private cursor and applies its flags.
    bool proceed(RayQueryState* state) const override
    {
        uint32_t& next = state->providerData[0];
        while (next < 3 && state->instanceInclusionMask)
        {
            const uint32_t index = next++;
            const bool triangle = index < 2;
            if (state->rayFlags & (triangle ? SLANG_RAY_QUERY_FLAG_SKIP_TRIANGLES
                                            : SLANG_RAY_QUERY_FLAG_SKIP_PROCEDURAL_PRIMITIVES))
                continue;

            RayQueryHit hit = {};
            hit.rayT = triangle ? float(2 - index) : 4.0f;
            hit.primitiveIndex = index;
            hit.barycentrics[0] = 0.25f;
            hit.barycentrics[1] = 0.5f;
            hit.proceduralPrimitiveNonOpaque = 1;
            for (uint32_t row = 0; row < 3; ++row)
            {
                const float scale = float(2u << row);
                hit.objectToWorld[row * 4 + row] = scale;
                hit.objectToWorld[row * 4 + 3] = scale;
                hit.worldToObject[row * 4 + row] = 1.0f / scale;
                hit.worldToObject[row * 4 + 3] = -1.0f;
            }
            if (triangle && !(state->rayFlags & SLANG_RAY_QUERY_FLAG_FORCE_NON_OPAQUE))
            {
                if (hit.rayT < state->committed.rayT)
                {
                    state->committed = hit;
                    state->committedStatus = SLANG_RAY_QUERY_COMMITTED_TRIANGLE_HIT;
                }
                continue;
            }
            state->candidate = hit;
            state->candidateType = triangle ? SLANG_RAY_QUERY_CANDIDATE_NON_OPAQUE_TRIANGLE
                                            : SLANG_RAY_QUERY_CANDIDATE_PROCEDURAL_PRIMITIVE;
            state->candidatePending = 1;
            state->traversalComplete = next == 3;
            return true;
        }
        state->candidatePending = 0;
        state->traversalComplete = 1;
        return false;
    }
};

// Exercises shader-side commits and accessors through compiled C++, including a final candidate
// yielded with traversal already complete. Compare all twelve elements in each matrix shape.
const char* kShader = R"(
RaytracingAccelerationStructure scene;
RWStructuredBuffer<float> output;

bool matricesMatch(float3x4 forward, float4x3 forwardTranspose,
                   float3x4 inverse, float4x3 inverseTranspose)
{
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 4; ++col)
        {
            float scale = float(2 << row);
            float f = col == row || col == 3 ? scale : 0.0;
            float i = col == row ? 1.0 / scale : col == 3 ? -1.0 : 0.0;
            if (forward[row][col] != f || forwardTranspose[col][row] != f ||
                inverse[row][col] != i || inverseTranspose[col][row] != i)
                return false;
        }
    return true;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint test = tid.x;
    uint flags = RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES;
    if (test == 3) flags |= RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    if (test == 4) flags = RAY_FLAG_SKIP_TRIANGLES;
    if (test == 5) flags |= RAY_FLAG_SKIP_TRIANGLES;
    if (test == 6) flags = RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES;
    RayDesc ray = { float3(0.0), 0.0, float3(0.0, 0.0, 1.0), 10.0 };
    RayQuery<RAY_FLAG_NONE> query;
    query.TraceRayInline(scene, flags, test == 7 ? 0 : 0xff, ray);
    uint count = 0;
    bool valid = true;
    while (query.Proceed())
    {
        ++count;
        valid = valid && matricesMatch(
            query.CandidateObjectToWorld3x4(), query.CandidateObjectToWorld4x3(),
            query.CandidateWorldToObject3x4(), query.CandidateWorldToObject4x3());
        if (query.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE)
        {
            valid = valid && all(query.CandidateTriangleBarycentrics() == float2(0.25, 0.5));
            if (test != 1) query.CommitNonOpaqueTriangleHit();
        }
        else
        {
            valid = valid && query.CandidateProceduralPrimitiveNonOpaque();
            query.CommitProceduralPrimitiveHit(asfloat(0x7fc00000u));
            query.CommitProceduralPrimitiveHit(-1.0);
            query.CommitProceduralPrimitiveHit(11.0);
            valid = valid && query.CommittedStatus() == COMMITTED_NOTHING;
            query.CommitProceduralPrimitiveHit(4.0);
            query.CommitProceduralPrimitiveHit(3.0);
            query.CommitProceduralPrimitiveHit(5.0);
        }
        if (test == 2) query.Abort();
    }
    bool hit = query.CommittedStatus() != COMMITTED_NOTHING;
    if (hit)
        valid = valid && matricesMatch(
            query.CommittedObjectToWorld3x4(), query.CommittedObjectToWorld4x3(),
            query.CommittedWorldToObject3x4(), query.CommittedWorldToObject4x3());
    output[test * 5] = query.CommittedStatus();
    output[test * 5 + 1] = hit ? query.CommittedPrimitiveIndex() : -1.0;
    output[test * 5 + 2] = hit ? query.CommittedRayT() : -1.0;
    output[test * 5 + 3] = count;
    output[test * 5 + 4] = valid;
}
)";
} // namespace

SLANG_UNIT_TEST(cpuRayQueryProviderRuntime)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    // Pin a native C++ compiler: direct LLVM emission does not implement RayQuery.
    const SlangPassThrough compilers[] = {
        SLANG_PASS_THROUGH_VISUAL_STUDIO,
        SLANG_PASS_THROUGH_GCC,
        SLANG_PASS_THROUGH_CLANG,
    };
    SlangPassThrough compiler = SLANG_PASS_THROUGH_NONE;
    for (auto candidate : compilers)
        if (SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(candidate)))
        {
            compiler = candidate;
            break;
        }
    if (compiler == SLANG_PASS_THROUGH_NONE)
        SLANG_IGNORE_TEST;
    globalSession->setDownstreamCompilerForTransition(
        SLANG_CPP_SOURCE,
        SLANG_SHADER_HOST_CALLABLE,
        compiler);

    slang::TargetDesc target = {};
    target.format = SLANG_SHADER_HOST_CALLABLE;
    slang::SessionDesc desc = {};
    desc.targetCount = 1;
    desc.targets = &target;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(desc, session.writeRef()) == SLANG_OK);
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "cpuRayQueryProvider",
        "cpuRayQueryProvider.slang",
        kShader,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);
    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(
        module->findEntryPointByName("computeMain", entryPoint.writeRef()) == SLANG_OK);
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(components, 2, program.writeRef()) == SLANG_OK);
    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);
    ComPtr<ISlangSharedLibrary> library;
    SLANG_CHECK_ABORT(
        linked->getEntryPointHostCallable(0, 0, library.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);
    using ComputeFunc = void (*)(ComputeVaryingInput*, void*, void*);
    auto compute = (ComputeFunc)library->findFuncByName("computeMain");
    SLANG_CHECK_ABORT(compute != nullptr);

    TestProvider provider;
    float results[8 * 5] = {};
    // Bind ordinary prelude values in shader-global declaration order, without an RHI device.
    struct Globals
    {
        RaytracingAccelerationStructure scene;
        RWStructuredBuffer<float> output;
    } globals = {{&provider}, {results, 8 * 5}};
    ComputeVaryingInput varying = {{0, 0, 0}, {8, 1, 1}};
    compute(&varying, nullptr, &globals);
    const float expected[8][5] = {
        {1, 1, 1, 2, 1},   // Commit the closest non-opaque triangle.
        {0, -1, -1, 2, 1}, // Reject all candidates.
        {1, 0, 2, 1, 1},   // Abort preserves the first committed hit.
        {1, 0, 2, 1, 1},   // Accept-first-hit ends traversal after one candidate.
        {2, 2, 3, 1, 1},   // Several procedural commits retain the closest valid distance.
        {0, -1, -1, 0, 1}, // Skip both geometry types.
        {1, 1, 1, 0, 1},   // Opaque triangles commit without yielding.
        {0, -1, -1, 0, 1}, // An empty instance mask skips traversal.
    };
    for (uint32_t test = 0; test < 8; ++test)
        for (uint32_t field = 0; field < 5; ++field)
            SLANG_CHECK(results[test * 5 + field] == expected[test][field]);
}

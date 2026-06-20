//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Regression test for a type-flow specialization crash:
//   "createSet called with non-global operands" (slang-ir.cpp).
//
// When a *generic* differentiable interface (`ISensor<Real>`) is loaded from a buffer and
// dispatched dynamically inside a `[Differentiable]` function that is then differentiated
// (`bwd_diff`), autodiff synthesizes extra witness tables for the interface, some of which live
// inside function blocks rather than at module scope. `collectExistentialTables` enumerated *all*
// witness tables of the interface type, including those block-local ones, and fed them to
// `IRBuilder::getSet` -- whose elements must be global (sets are canonicalized by global unique
// id). That asserted.
//
// The fix restricts `collectExistentialTables` to module-scope (global) witness tables, matching
// the intent of its caller `findGlobalTables` and the `getSet` invariant. The block-local tables
// are derivative artifacts, not primal conformances, so excluding them preserves the type set.

[anyValueSize(16)]
interface ISensor<Real : IFloat> : IDifferentiable
{
    [Differentiable] Real splat(Real p);
}

struct Perspective<Real : IFloat> : ISensor<Real>, IDifferentiable
{
    Real focal;
    [Differentiable] Real splat(Real p) { return focal * p; }
}

// A second, never-instantiated conformance keeps the dispatch dynamic (not devirtualized).
struct Ortho<Real : IFloat> : ISensor<Real>, IDifferentiable
{
    Real scale;
    [Differentiable] Real splat(Real p) { return scale + p; }
}

//TEST_INPUT: type_conformance Perspective<float>:ISensor<float> = 0
//TEST_INPUT: type_conformance Ortho<float>:ISensor<float> = 1

// One ISensor<float> box: header RTTI[0 0] witness[0 0] (Perspective, seq 0), payload focal=3.0
//TEST_INPUT: set sensors = ubuffer(data=[0 0 0 0 1077936128 0 0 0], stride=4)
StructuredBuffer<ISensor<float>> sensors;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// A separate `[Differentiable]` function that takes the existential by value and dispatches a
// differentiable method on it -- this is what drives the interprocedural type-flow analysis.
[Differentiable]
float project(ISensor<float> sensor, float p)
{
    return sensor.splat(p);
}

[Differentiable]
float compute(int i, float p)
{
    ISensor<float> s = sensors[i];
    return project(s, p);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var dpp = diffPair(2.0f);
    bwd_diff(compute)(0, dpp, 1.0);
    // d/dp [ focal * p ] = focal = 3.0
    // CHECK: 3.0
    outputBuffer[tid.x] = dpp.d;
}

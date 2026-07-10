//TEST:SIMPLE(filecheck=PRUNED): -target hlsl -entry computeMain -stage compute -dump-ir
//TEST:SIMPLE(filecheck=KEPT): -target hlsl -entry computeMain -stage compute -dump-ir -DUSE_AUTODIFF

// Regression test for auto-diff link-time pruning
// (`IRSharedSpecContext::canPruneAutodiffLinkArtifacts` in slang-ir-link.cpp),
// the fix for the PR #9808 compile-time regression (issue #11781).
//
// The PRUNED variant never differentiates, but calls a differentiable builtin
// (`sin`) whose witness tables carry the auto-diff machinery: the final
// codegen link must not clone any module-scope `IRAnnotation` into it. The
// no-autodiff IR dump is asserted annotation-free after the entry point first
// appears — under the pre-fix behavior the linked module contains hundreds of
// `Annotation(...)` insts here.
//
// The KEPT variant compiles the same source with an actual `__fwd_diff` call,
// pinning that (a) the gate does not prune when auto-diff is in use, and
// (b) the `Annotation(` spelling the PRUNED-NOT relies on is still what the
// IR dump emits (guarding the negative check against vacuity).

// Note the `[ForwardDifferentiable]` helper must sit inside the ifdef too:
// merely *defining* a differentiable function generates module-local
// annotations (e.g. `NullDifferential`) during lower-to-ir, independent of the
// link-time gate this test pins.
#ifdef USE_AUTODIFF
typedef DifferentialPair<float> dpfloat;

[ForwardDifferentiable]
float square(float x)
{
    return x * x;
}
#endif

RWStructuredBuffer<float> outputBuffer;

// PRUNED: computeMain
// PRUNED-NOT: Annotation(

// KEPT: computeMain
// KEPT: Annotation(

[numthreads(1, 1, 1)]
void computeMain()
{
#ifdef USE_AUTODIFF
    outputBuffer[0] = __fwd_diff(square)(dpfloat(3.0, 1.0)).d;
#else
    outputBuffer[0] = sin(outputBuffer[1]);
#endif
}

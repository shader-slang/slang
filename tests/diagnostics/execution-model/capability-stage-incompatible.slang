// Diagnostic: a function requiring `fragment` stage capability is
// called from a `compute` entry point. The capability sets are
// stage-incompatible and the compiler must report it.
//
// Exercises slang-capability.cpp's set-algebra reasoning around
// `isIncompatibleWith` (and the `concurrentIterate` template
// helpers) which are not exercised by the PR's existing
// `[shader("invalid")]` test (that test short-circuits at the
// "unknown atom" check, never reaching the set algebra).

// Use non-exhaustive mode: we want to pin the stage-incompatibility
// error specifically (E36107) without enumerating every related
// note / span that the diagnostic system emits.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -emit-spirv-directly -entry computeMain -stage compute

// CHECK: E36107
// CHECK: unavailable features in entry point

// `[require(fragment)]` restricts this function to the fragment
// stage.
[require(fragment)]
float fragmentOnlyHelper(float x)
{
    return x * 2.0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Calling a fragment-only helper from compute exercises the
    // capability-set-incompatibility path in slang-capability.cpp.
    float v = fragmentOnlyHelper(1.0);
}

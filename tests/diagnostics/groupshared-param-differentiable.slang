// A `groupshared` parameter is passed by reference, so a differentiable function cannot propagate a
// derivative back through it. Both the read-write and the `const` spelling are rejected, and
// `no_diff` does not change that -- the parameter is still passed by reference. Without this
// diagnostic each case reaches an unconditional abort in the backward-diff type builders
// (shader-slang/slang#10641).
//
// A parameter whose type is not differentiable is unaffected: `uint[N]` does not conform to
// `IDifferentiable`, so no derivative is expected through it in the first place.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

static const uint N = 8;
groupshared float sharedFloats[N];
groupshared uint sharedUInts[N];

[Differentiable]
float readWrite(uint tid, groupshared float s[N])
//CHECK:                  ^^^^^^^^^^^ error E38038
{
    return s[tid];
}

[Differentiable]
float readOnly(uint tid, const groupshared float s[N])
//CHECK:                       ^^^^^^^^^^^ error E38038
{
    return s[tid];
}

// `no_diff` does not help: the parameter is still passed by reference.
[Differentiable]
float noDiff(uint tid, no_diff const groupshared float s[N])
//CHECK:                             ^^^^^^^^^^^ error E38038
{
    return s[tid];
}

// A non-differentiable element type is accepted -- `uint[N]` does not conform to `IDifferentiable`.
[Differentiable]
float nonDifferentiableElement(uint tid, const groupshared uint s[N])
{
    return float(s[tid]);
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    sharedFloats[tid.x] = float(tid.x);
    sharedUInts[tid.x] = tid.x;
}

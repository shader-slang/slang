// A `groupshared` parameter of a differentiable function is passed by reference, which autodiff
// cannot currently lower, so both the bare and `const groupshared` spellings are rejected. A
// `no_diff` parameter (excluded from differentiation) and a non-differentiable element type (no
// derivative expected) need no gradient through the parameter and compile.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

static const uint N = 8;
groupshared float sharedFloats[N];
groupshared uint sharedUInts[N];

// Rejected: a bare (read-write) `groupshared` parameter with a differentiable element type.
[Differentiable]
float readWrite(uint tid, groupshared float s[N])
//CHECK:                  ^^^^^^^^^^^ error E38038
{
    return s[tid];
}

// Rejected: a read-only `const groupshared` parameter with a differentiable element type is likewise
// unlowerable in backward-diff today.
[Differentiable]
float readOnly(uint tid, const groupshared float s[N])
//CHECK:                       ^^^^^^^^^^^ error E38038
{
    return s[tid];
}

// Accepted: `no_diff` excludes the parameter from differentiation, so its passing mode is irrelevant.
[Differentiable]
float noDiff(uint tid, no_diff const groupshared float s[N])
{
    return s[tid];
}

// Accepted: a non-differentiable element type expects no derivative -- `uint[N]` does not conform to
// `IDifferentiable`.
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

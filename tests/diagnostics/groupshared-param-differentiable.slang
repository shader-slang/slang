// A `groupshared` parameter of a differentiable function is passed by reference, and autodiff
// propagates no derivative through a reference, so both the bare and `const groupshared` spellings
// are treated as `no_diff`. Because a user may not expect that, we warn unless the parameter is
// explicitly marked `no_diff` or its element type has no derivative to lose.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

static const uint N = 8;
groupshared float sharedFloats[N];
groupshared uint sharedUInts[N];

// Warned: a bare (read-write) `groupshared` parameter with a differentiable element type.
[Differentiable]
float readWrite(uint tid, groupshared float s[N])
//CHECK:                  ^^^^^^^^^^^ 'groupshared' parameter of a differentiable function is treated as 'no_diff'
//CHECK:                  ^^^^^^^^^^^ a 'groupshared' parameter is passed by reference, so no derivative is propagated through it; mark it 'no_diff' to make that explicit.
{
    return s[tid];
}

// Warned: a read-only `const groupshared` parameter with a differentiable element type.
[Differentiable]
float readOnly(uint tid, const groupshared float s[N])
//CHECK:                       ^^^^^^^^^^^ 'const groupshared' parameter of a differentiable function is treated as 'no_diff'
//CHECK:                       ^^^^^^^^^^^ a 'groupshared' parameter is passed by reference, so no derivative is propagated through it; mark it 'no_diff' to make that explicit.
{
    return s[tid];
}

// Not warned: the user already wrote `no_diff`.
[Differentiable]
float noDiff(uint tid, no_diff const groupshared float s[N])
{
    return s[tid];
}

// Not warned: a non-differentiable element type has no derivative to lose -- `uint[N]` does not
// conform to `IDifferentiable`.
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

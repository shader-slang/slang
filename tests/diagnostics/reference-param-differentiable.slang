// A reference parameter of a differentiable function -- `__ref`, `__ref_readonly`,
// `__ref_writeonly`, `groupshared`, or `const groupshared` -- names storage that the caller, and
// for `groupshared` other invocations, may also access. Autodiff propagates no derivative through
// a reference, so the parameter is treated as `no_diff`. Because a user may not expect that, we
// warn unless the parameter is explicitly marked `no_diff`, its type has no derivative to lose,
// or it is `groupshared`. Differentiating a function with a `groupshared` parameter is an error
// at the call, so the function's derivative never exists to lose one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute -experimental-feature

static const uint N = 8;
groupshared float sharedFloats[N];
groupshared uint sharedUInts[N];

// Warned: each explicit reference keyword.
[Differentiable]
float refParam(__ref float a, float x)
//CHECK:                   ^ parameter 'a' is passed by reference, so a differentiable function treats it as 'no_diff'
//CHECK:                   ^ no derivative is propagated through a reference parameter; mark it 'no_diff' to make that explicit.
{
    return a * x;
}

[Differentiable]
float refReadOnlyParam(__ref_readonly float a, float x)
//CHECK:                                    ^ parameter 'a' is passed by reference, so a differentiable function treats it as 'no_diff'
//CHECK:                                    ^ no derivative is propagated through a reference parameter; mark it 'no_diff' to make that explicit.
{
    return a * x;
}

[Differentiable]
float refWriteOnlyParam(__ref_writeonly float a, float x)
//CHECK:                                      ^ parameter 'a' is passed by reference, so a differentiable function treats it as 'no_diff'
//CHECK:                                      ^ no derivative is propagated through a reference parameter; mark it 'no_diff' to make that explicit.
{
    a = detach(x);
    return x;
}

// Not warned: `groupshared` parameters, read-write and read-only, with a differentiable element
// type.
[Differentiable]
float readWrite(uint tid, groupshared float s[N])
{
    return s[tid];
}

[Differentiable]
float readOnly(uint tid, const groupshared float s[N])
{
    return s[tid];
}

// Not warned: the user already wrote `no_diff`.
[Differentiable]
float noDiff(uint tid, no_diff const groupshared float s[N])
{
    return s[tid];
}

[Differentiable]
float noDiffRef(no_diff __ref float a, float x)
{
    return a * x;
}

// Not warned: a non-differentiable type has no derivative to lose -- neither `uint[N]` nor `int`
// conforms to `IDifferentiable`.
[Differentiable]
float nonDifferentiableElement(uint tid, const groupshared uint s[N])
{
    return float(s[tid]);
}

[Differentiable]
float nonDifferentiableRef(__ref int a, float x)
{
    return a * x;
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    sharedFloats[tid.x] = float(tid.x);
    sharedUInts[tid.x] = tid.x;
}

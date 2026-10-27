// A non-differentiable `[ForceInline]` helper is inlined into a differentiable function before
// autodiff prepares the function's body, so a write in the helper to part of
// thread-group-shared memory becomes a partial write in the differentiated body and races like
// any other. The write is in the helper's body, though, which the check of the differentiable
// function does not see, and the helper alone does not know that it will be inlined into
// differentiated code.
//
// Disabled: we do not check the bodies of inlined helpers. The annotation matches only the
// severity, because the expected diagnostic and its location depend on how such a check is
// designed.

//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

[ForceInline]
float bumpInline(__ref float a)
{
    a += 1.0;
    return a;
}

[Differentiable]
float throughForceInlineNonDifferentiable(uint i, float x)
{
    return x * no_diff(bumpInline(g[i]));
//CHECK:                              ^ warning
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outputBuffer[i] = throughForceInlineNonDifferentiable(i, 1.0);
}

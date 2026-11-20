// Regression test for https://github.com/shader-slang/slang/issues/14548.
//
// A [require(...)] on a variable declaration — a free global static const, or a
// static const data member of a struct, generic or not — was silently unenforced:
// `SemanticsDeclCapabilityVisitor::checkVarDeclCommon` seeded the var's inferred
// capability set from its referenced-type walk but never merged the var's own
// [require] through `getDeclaredCapabilitySet`, unlike the function path in
// `checkFunctionCommon`. A consumer compiled for a target the variable does not
// support therefore compiled clean. Each entry point below references a
// `spirv`-gated variable and is compiled for `hlsl`, so every invocation must
// fail with E36107. The last invocation is the control case: a gated function
// consumed in the same position was already enforced.

// free global static const
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKVAR,non-exhaustive):-target hlsl -entry computeMainVar -stage compute
// static const data member, non-generic spelling
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKMEMBER,non-exhaustive):-target hlsl -entry computeMainMember -stage compute
// static const data member reached through a generic spelling
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKGENERIC,non-exhaustive):-target hlsl -entry computeMainGeneric -stage compute
// internal gated variable: the internal join branch (declared joins inferred, no
// superset enforcement) must gate consumers the same way
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKINTERNAL,non-exhaustive):-target hlsl -entry computeMainInternal -stage compute
// control: a gated function consumed in the same position
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKFUNC,non-exhaustive):-target hlsl -entry computeMainFunc -stage compute

[require(spirv)]
static const float w = 1.0;

[require(spirv)]
internal static const float wInternal = 1.0;

struct G
{
    [require(spirv)]
    static const float v = 1.0;
};

struct GT<T>
{
    [require(spirv)]
    static const float v = 1.0;
};

[require(spirv)]
float f()
{
    return 1.0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainVar(uint3 tid : SV_DispatchThreadID)
{
    if (w > 0.0)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainMember(uint3 tid : SV_DispatchThreadID)
{
    if (G.v > 0.0)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainGeneric(uint3 tid : SV_DispatchThreadID)
{
    if (GT<float>.v > 0.0)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainInternal(uint3 tid : SV_DispatchThreadID)
{
    if (wInternal > 0.0)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainFunc(uint3 tid : SV_DispatchThreadID)
{
    if (f() > 0.0)
    {
    }
}

//CHECKVAR: E36107
//CHECKMEMBER: E36107
//CHECKGENERIC: E36107
//CHECKINTERNAL: E36107
//CHECKFUNC: E36107

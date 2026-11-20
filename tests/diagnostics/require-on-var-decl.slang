// Regression test for https://github.com/shader-slang/slang/issues/14548.
//
// A [require(...)] on a variable declaration — a free global static const, or a
// static const data member of a struct, generic or not — was silently unenforced:
// `SemanticsDeclCapabilityVisitor::checkVarDeclCommon` walked the variable's type
// into its inferred capability set but never merged the variable's own [require]
// attributes, so both the variable and every consumer of it compiled cleanly on
// unsupported targets. The fix mirrors the declared-set handling of
// `SemanticsDeclCapabilityVisitor::visitFunctionDeclBase`: public decls must
// declare a superset of what their type references; internal decls join the two
// sets. Only the variable's own attributes are merged — deliberately not the
// parent chain, which would feed the enclosing declaration's requirements back
// into the per-`target_switch`-case capability computation through parameter and
// local references.
//
// The first four invocations are the issue's repro shapes plus the internal-join
// branch; each expects the consumer-side E36107. CHECKOK is the positive control:
// the same gated-variable consumer compiles cleanly for a supporting target.
// CHECKFUNC is the control case: a gated function consumed in the same position
// was already enforced before the fix. Every failing invocation is non-exhaustive
// because each error also emits provenance notes ("see using/definition of ...")
// that are intentionally left unmatched.

// free global static const
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKVAR,non-exhaustive):-target hlsl -entry computeMainVar -stage compute
// static const data member, non-generic spelling
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKMEMBER,non-exhaustive):-target hlsl -entry computeMainMember -stage compute
// static const data member reached through a generic spelling
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKGENERIC,non-exhaustive):-target hlsl -entry computeMainGeneric -stage compute
// internal gated variable: the internal join branch gates consumers too
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKINTERNAL,non-exhaustive):-target hlsl -entry computeMainInternal -stage compute
// positive control: the gated-variable consumer compiles for a supporting target
//TEST:SIMPLE(filecheck=CHECKOK): -target spirv -entry computeMainVar -stage compute
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
//CHECKVAR: see definition of 'w'
//CHECKMEMBER: E36107
//CHECKMEMBER: see definition of 'v'
//CHECKGENERIC: E36107
//CHECKGENERIC: see definition of 'v'
//CHECKINTERNAL: E36107
//CHECKINTERNAL: see definition of 'wInternal'
//CHECKOK-NOT: 36107
//CHECKFUNC: E36107
//CHECKFUNC: see definition of 'f'

// Decl-site enforcement for [require(...)] on variable declarations, companion to
// require-on-var-decl.slang (https://github.com/shader-slang/slang/issues/14548).
//
// The checks here fire at the declaration or its immediate use, no matter which
// entry point is compiled, which is why they live in their own file — a
// must-compile positive control in the same file would see these errors too.
//
// CHECKSUBSET pins the public-superset diagnostic: a variable whose own [require]
// excludes what its type references is rejected at its declaration (E36108). Its
// entry declares {hlsl | spirv} so the gated reference is covered at declaration
// checking; the consumer-side failure for that entry is the E36107 of referencing
// a spirv-gated variable from an hlsl compilation.
//
// CHECKDESIGN pins the own-attributes-only merge: a spirv-gated parameter
// referenced from an hlsl `target_switch` case produces a per-case capability
// conflict (E36101). If the merge instead walked the parent chain
// (getDeclaredCapabilitySet), the parameter's set would widen to the enclosing
// function's {hlsl | spirv} and the same conflict would compile silently — so this
// case fails exactly when the declared-capability merge is in place and scoped to
// the variable's own attributes.
//
// CHECKNARROW pins the accepted asymmetry with the member-function path: a static
// member whose own [require] narrows below its parent's conflicts at every use on
// every target (E36100), because the base-expression walk joins the parent's set
// with the member's own — where the member-function path joins the parent's set
// into the declared set and yields a usable union.
//
// CHECKEXT pins the extension-member conflict check for data members: an extension
// member variable whose own [require] is incompatible with the extension's target
// type is diagnosed at the declaration, the same way member functions, subscripts
// and properties are.
//
// Invocations are non-exhaustive because every compilation of this file produces
// all of these errors (and their provenance notes), while each invocation pins
// only its own; an exhaustive check would fail on every unannotated diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKSUBSET,non-exhaustive):-target hlsl -entry computeMainSubset -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKDESIGN,non-exhaustive):-target hlsl -entry computeMainDesign -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKNARROW,non-exhaustive):-target hlsl -entry computeMainNarrow -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKEXT,non-exhaustive):-target hlsl -entry computeMainExt -stage compute

[require(metal)]
struct M
{
    uint x;
};

// Public decl whose type requires `metal` while it declares only `spirv`.
[require(spirv)]
static const M mv = M();

[require(metal)]
struct MNarrow
{
    [require(hlsl)]
    static const float v = 1.0;
};

[require(glsl)]
struct SExt
{
    int x;
}

extension SExt
{
    [require(hlsl)]
    static const int sExtVar = 1;
}

[require(hlsl)]
[require(spirv)]
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainSubset(uint3 tid : SV_DispatchThreadID)
{
    if (mv.x > 0)
    {
    }
}

[require(hlsl)]
[require(spirv)]
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainDesign([require(spirv)] uint3 tid : SV_DispatchThreadID)
{
    __target_switch
    {
    case hlsl:
        if (tid.x > 0)
        {
        }
    case spirv:
        if (tid.y > 0)
        {
        }
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainNarrow(uint3 tid : SV_DispatchThreadID)
{
    if (MNarrow.v > 0.0)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainExt(uint3 tid : SV_DispatchThreadID)
{
    if (SExt.sExtVar > 0)
    {
    }
}

//CHECKSUBSET: E36108
//CHECKDESIGN: E36101
//CHECKNARROW: E36100
//CHECKEXT: E36100
//CHECKEXT: conflicting with the 'sExtVar'

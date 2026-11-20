// Decl-site enforcement for [require(...)] on variable declarations, companion to
// require-on-var-decl.slang (https://github.com/shader-slang/slang/issues/14548).
//
// The two checks here are decl-site: they fire when the declaration itself is
// checked, no matter which entry point is compiled, which is why they live in their
// own file — a must-compile positive control in the same file would see these
// errors too.
//
// CHECKSUBSET pins the public-superset diagnostic: a variable whose own [require]
// excludes what its type references is rejected at its declaration (E36108).
//
// CHECKDESIGN pins the own-attributes-only merge: a spirv-gated parameter referenced
// from an hlsl `target_switch` case produces a per-case capability conflict
// (E36101). If the merge instead walked the parent chain (getDeclaredCapabilitySet),
// the parameter's set would widen to the enclosing function's {hlsl | spirv} and the
// same conflict would compile silently — so this case fails exactly when the
// declared-capability merge is in place and scoped to the variable's own
// attributes. Invocations are non-exhaustive because both decl-site errors appear
// in every compilation of this file, and each error also emits provenance notes
// that are intentionally left unmatched.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKSUBSET,non-exhaustive):-target hlsl -entry computeMainSubset -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKDESIGN,non-exhaustive):-target hlsl -entry computeMainDesign -stage compute

[require(metal)]
struct M
{
    uint x;
};

// Public decl whose type requires `metal` while it declares only `spirv`.
[require(spirv)]
static const M mv = M();

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

//CHECKSUBSET: E36108
//CHECKDESIGN: E36101

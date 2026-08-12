//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#capability--target-attributes
//META: doc_section_digest=3a5e5ccd3c56586ff535ba4ba12aa25b1d20542d755d9561cb4e0be92dc6ca73
//META: purpose=`[require(capability)]` makes a call from an entry point that lacks the capability a checking error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the attribute is enforced rather than advisory: the same function
// that compiles for SPIR-V is rejected when the entry point is compiled for HLSL,
// because the required atom is not in that target's capability set. CHECK pins
// the unavailable-features error code reported on the entry point.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[require(spirv)]
int spvOnly(int x)
{
    return x + 1;
}

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    //CHECK: E36107
    outBuf[tid.x] = spvOnly(int(tid.x));
}

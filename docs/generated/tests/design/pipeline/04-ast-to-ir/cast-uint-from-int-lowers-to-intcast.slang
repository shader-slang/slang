//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=An explicit constructor-style cast between two integer widths/signednesses (`uint(intValue)`) lowers to an `intCast` IR opcode — the integer-to-integer counterpart of `castFloatToInt`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: explicit-cast axis — integer-to-integer. The existing
// `conversion-explicit-cast-...` test covers float<->int conversions;
// `uint(int_x)` is the same pure-value-inst family but a different
// IR opcode (`intCast`) because both source and dest are integral.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint u = uint(a);
    buf[0] = u;
}

// CHECK-LABEL: func %main
// CHECK: intCast(%{{.*}})

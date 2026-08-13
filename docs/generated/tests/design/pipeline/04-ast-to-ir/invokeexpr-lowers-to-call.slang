//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=An InvokeExpr (general operator / function call) lowers, after callable resolution, to an `IRCall` (spelled `call %<callee>(...)` in the dump).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Calling a user function lowers to a `call %doIt(...)` instruction.
// The doc's row: "InvokeExpr (general operator / function call) →
// IRCall (after callable resolution)". The dump's
// opcode spelling is the lowercase Lua name `call`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int doIt(int v) { return v * 2; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = doIt(a);
}

// CHECK-LABEL: func %main
// CHECK: call %doIt(%{{.*}})

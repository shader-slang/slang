//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=19eb09c072c556dd0799b5002c2f1b24781da93f2cebc122ca027062c24e0b84
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

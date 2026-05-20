//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A `return` from a void-returning function lowers to `return_val(void_constant)` — the doc's "ReturnStmt → IRReturn terminator" claim, void variant.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc maps `ReturnStmt` to `IRReturn`. For a void-returning
// function the operand is `void_constant`, a distinguished constant
// inst. Observable for the entry-point `void main(...)` directly.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 1;
}

// CHECK-LABEL: func %main
// CHECK: return_val(void_constant)

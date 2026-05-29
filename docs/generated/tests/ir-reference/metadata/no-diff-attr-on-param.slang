//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#attr-family
//META: doc_section_digest=f3430c3e5f3d15f08a7b508e5bb6b1ba5106fa40b74c0d99d17989a7e31e58a8
//META: purpose=The no_diff Attr opcode is created from the NoDiffModifier on a function parameter and surfaces as a bare let-binding in the IR dump.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Attr-family table: `no_diff` is a no-operand hoistable
// attribute originating from `NoDiffModifier` (autodiff-related).
// It appears in the dump as `= no_diff` on a void-typed `let`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

float helper(no_diff float x)
{
    return x * 2.0;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helper(float(tid.x));
}

// CHECK: = no_diff

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A uniform `uint` plus the literal `1u` lowers to a runtime `add(%a, 1 : UInt)` instruction; lowering does not pre-compute the sum because one operand is non-constant.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: integer-arithmetic at the uint MAX + 1 edge, runtime
// form. Because `%a` is a uniform (not a compile-time literal), the
// IRBuilder cannot constant-fold `a + 1u` and emits an `add`
// instruction whose RHS is the literal `1 : UInt`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint b = a + 1u;
    buf[0] = b;
}

// CHECK-LABEL: func %main
// CHECK: add(%a, 1 : UInt)

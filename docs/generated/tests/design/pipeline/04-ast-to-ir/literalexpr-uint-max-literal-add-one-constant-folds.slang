//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=When both operands of `+` are compile-time integer literals (uint MAX and `1u`), the IRBuilder constant-folds at lowering and the body emits `store(..., 0 : UInt)` directly — no `add` instruction survives.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: integer-arithmetic at uint MAX + 1, constant-fold path.
// `uint(0xFFFFFFFF) + 1u` is fully literal; the doc's "LiteralExpr
// → constant inst" entry combined with hoistable-value
// deduplication means the IRBuilder folds the sum to `0 : UInt`
// before any `add` is emitted. Verifies that no `add(` survives in
// `main` (with a separator `store`).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint a = uint(0xFFFFFFFF);
    uint b = a + 1u;
    buf[0] = b;
}

// CHECK-LABEL: func %main
// CHECK-NOT: add(
// CHECK: store({{.*}}, 0 : UInt)

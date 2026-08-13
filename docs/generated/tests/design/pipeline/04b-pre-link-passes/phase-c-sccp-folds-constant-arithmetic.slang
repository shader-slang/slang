//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes
//META: doc_section_digest=49feefd921e3cd705ce44340fcdfa68e653acc4e8992a6e7baac1b7332480921
//META: purpose=Phase C's applySparseConditionalConstantPropagation (C2) runs over the freshly constructed SSA and folds constant arithmetic to a single literal.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the constant-propagation step by giving it literal operands
// only: `7` flows into `x`, `x + 3` is therefore statically known, and the
// post-Phase-D snapshot should show the folded `10` being stored rather
// than an `add` of the two source literals. The CHECK-NOT pins the second
// half of that -- the unfolded add must not be present.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = 7;
    int y = x + 3;
    buf[0] = y;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-LABEL: func %main
// CHECK: store({{.*}}, 10 : Int)
// CHECK-NOT: add(7 : Int, 3 : Int)

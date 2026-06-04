//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes
//META: doc_section_digest=b2bba5b4923f9f3b4fa76f2dbfcef4a05d4c5bbf61e50737eaabe8565887e46f
//META: purpose=Phase C's applySparseConditionalConstantPropagation (C2) folds `7 + 3` to a single constant 10 stored into the buffer.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SCCP runs after SSA construction and constant-folds the
// arithmetic; the LOWER-TO-IR block shows the folded literal
// instead of `add(7, 3)`.

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

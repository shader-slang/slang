//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes
//META: doc_section_digest=49feefd921e3cd705ce44340fcdfa68e653acc4e8992a6e7baac1b7332480921
//META: purpose=Phase C's per-function eliminateDeadCode sweep (C5) removes instructions whose results are never used, while keepExportsAlive leaves the entry point itself in place.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-function DCE step. The multiply is fed from a `uniform`
// so constant folding cannot claim credit for removing it; the only reason
// it can disappear is that nothing reads `dead`. The CHECK-NOT looks for
// the distinctive multiplier literal inside `main`, and the surviving
// `func %main` label shows the sweep's keep-exports-alive setting did not
// take the entry point with it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int dead = a * 999;
    buf[0] = 1;
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-LABEL: func %main
// CHECK-NOT: 999 : Int

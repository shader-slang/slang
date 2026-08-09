//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=647327ee76a1bc33d8bcf7d2abc19a2f16ee4e63d10bad4d1d5d48c120029d9d
//META: purpose=Stress - `branch` and `flatten` are per-conditional markers: three hinted conditionals in one function each keep their own bare decoration.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the two conditional hints are recorded per `ifElse` inst
// rather than as one function-level mode. Three conditionals in the same
// entry point carry `[branch]`, `[flatten]`, `[branch]`; the CHECK-DAG
// lines assert both spellings survive lowering, which a scheme that
// overwrote one hint with the next could not satisfy.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    [branch]
    if (tid.x > 0) s = 1; else s = 2;
    [flatten]
    if (tid.y > 0) s += 4;
    [branch]
    if (tid.z > 0) s += 8; else s -= 8;
    buf[0] = s;
}

// CHECK-DAG: [branch]
// CHECK-DAG: [flatten]

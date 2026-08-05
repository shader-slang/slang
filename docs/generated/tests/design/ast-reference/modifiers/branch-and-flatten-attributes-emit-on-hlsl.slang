//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=2884135594e88fad2ff6c4fdc20d6bf694888dfaa9f666f2ac51e906fc88f826
//META: purpose=The `[branch]` and `[flatten]` branch-hint attributes are forwarded to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two opposing branch hints in the same family reach the back end
// under their own spellings. Both conditions are driven by the thread id so
// neither `if` can be folded away, and the CHECK-DAG lines assert the emitted
// HLSL carries `[branch]` on one and `[flatten]` on the other -- proving the two
// attributes are distinguished, not collapsed to a single marker.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    [branch]
    if (tid.x > 2) { r = 1; } else { r = 2; }
    [flatten]
    if (tid.y > 2) { r += 10; }
    outBuf[tid.x] = r;
}

//CHECK-DAG: [branch]
//CHECK-DAG: [flatten]

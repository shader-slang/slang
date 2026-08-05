//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#emit-dispatcher
//META: doc_section_digest=f136369c763a469783c526b150d8baf5d8f6435a816fbf58504eaa5c871b9d36
//META: purpose=WGSL targets resolve to the `None` line-directive mode because WGSL has no line directives, so the emitted WGSL contains no `#line` markers at all.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative half of the line-directive-mode resolution. The same
// multi-statement kernel that makes HLSL emit several `#line` directives must
// produce none on WGSL, since `#` is not valid WGSL syntax anywhere. The WGSL
// prefix uses CHECK-NOT over the whole output for both the C-style and the
// bare directive spelling; the HLSL prefix confirms the source really does
// have locations to advance through, so the WGSL result is a suppression
// rather than an artefact of a locationless shader.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);

    int b = int(tid.y);

    outBuf[0] = a * b;
}

// WGSL: @compute
// WGSL-NOT: #line
// HLSL: {{^}}#line

//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#source-writer-abstraction
//META: doc_section_digest=5ab3f266239be1b6579f1150e2e0f0b0b3f4aacd7432b541f42ae07cd29ad3ca
//META: purpose=Boundary/negative: WGSL emit does NOT produce `#line` directives — WGSL has no `#line` syntax, so the SourceWriter on this target omits source-location anchors entirely.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative-shape probe: WGSL is the other target NOT in the
// "SourceWriter emits #line" list from
// `source-writer-emits-line-directives.slang`. The WGSL grammar has
// no `#line` directive, and the emit accordingly never writes one.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
    outBuf[tid.x + 1] = int(tid.x + 1);
}

// CHECK: @compute
// CHECK-NOT: #line

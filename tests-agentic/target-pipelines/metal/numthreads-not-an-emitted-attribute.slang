//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Metal expresses the threadgroup size at dispatch time, so `[numthreads(X,Y,Z)]` does not survive as a `numthreads(...)` attribute in the emitted MSL (contrast with HLSL).
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[numthreads(8,4,2)]` is HLSL-specific. On Metal the
// dimensions come from the dispatch call, so no `numthreads`
// token appears in the emitted Metal text. The kernel just
// carries the standard `[[thread_position_in_grid]]` entry
// parameter.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: thread_position_in_grid
// CHECK-NOT: numthreads(

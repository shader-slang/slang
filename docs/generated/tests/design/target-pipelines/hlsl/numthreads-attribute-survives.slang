//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
//META: purpose=A compute entry point with [numthreads(X,Y,Z)] survives the pipeline as a [numthreads(X, Y, Z)] HLSL attribute on the entry point function.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The HLSL source emitter writes the entry-point's numthreads
// dimensions as the canonical `[numthreads(X, Y, Z)]` attribute
// immediately before `void main(...)`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: numthreads(8, 4, 2)
// CHECK: void main(uint3 tid_{{[0-9]+}} : SV_DispatchThreadID)

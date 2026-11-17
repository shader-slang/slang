//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
//META: purpose=Boundary: an explicit cs_6_0 (lowest documented SM 6.x compute profile) still emits the canonical HLSL prelude pack_matrix(column_major).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: profile lower edge — `-profile cs_6_0` is the
// lowest shader-model-6 compute profile. The emitter still
// writes the standard prelude.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -profile cs_6_0

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #pragma pack_matrix(column_major)

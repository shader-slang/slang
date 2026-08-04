//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
//META: purpose=Stress: a fixed-size array of eight RWStructuredBuffers consumes register slots u0..u7 and the trailing scalar UAV gets u8.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: resource arrays of length 8. The emitter packs
// the array starting at u0, and a subsequent scalar UAV picks
// up the next available slot.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> bufs[8];
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    [unroll]
    for (int i = 0; i < 8; i++)
        s += bufs[i][tid.x];
    outBuf[tid.x] = s;
}

// CHECK: RWStructuredBuffer<int{{.*}}>{{.*}}bufs_{{[0-9]+}}{{.*}}register(u0)
// CHECK: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}} : register(u8)

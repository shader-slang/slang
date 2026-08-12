//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
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

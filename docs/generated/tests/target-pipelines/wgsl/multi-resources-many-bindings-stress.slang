//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=287321dc153823860396c030325e4ad4c0f6064005fa37b19c8ba53087dcd3e6
//META: purpose=Eight module-scope storage buffers receive eight distinct sequential @binding indices in @group(0)
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe of the auto-binding-assignment path: eight
// resources all share @group(0) and each gets its own @binding
// index.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;
StructuredBuffer<float> a0;
StructuredBuffer<float> a1;
StructuredBuffer<float> a2;
StructuredBuffer<float> a3;
StructuredBuffer<float> a4;
StructuredBuffer<float> a5;
StructuredBuffer<float> a6;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = a0[tid.x] + a1[tid.x] + a2[tid.x] + a3[tid.x]
                  + a4[tid.x] + a5[tid.x] + a6[tid.x];
}

// CHECK: @binding(0) @group(0)
// CHECK: @binding(1) @group(0)
// CHECK: @binding(2) @group(0)
// CHECK: @binding(3) @group(0)
// CHECK: @binding(4) @group(0)
// CHECK: @binding(5) @group(0)
// CHECK: @binding(6) @group(0)
// CHECK: @binding(7) @group(0)

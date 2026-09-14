//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=7d3641e4955ae039b13fce50516f0ed39854eeed3ca279f81a665ace48b92daa
//META: purpose=A struct field after a vec3 carries @align(4) in the std430 wrapper while the vec3 itself is @align(16)
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL aligns vec3 to 16 bytes, so a trailing scalar field needs
// distinct @align markers in the std430 wrapper struct emitted for
// a structured buffer.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

struct VecStruct
{
    float3 v;
    float trailing;
};

RWStructuredBuffer<VecStruct> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    VecStruct s;
    s.v = float3(1.0f, 2.0f, 3.0f);
    s.trailing = 9.0f;
    outBuf[tid.x] = s;
}

// CHECK: struct VecStruct_std430
// CHECK: @align(16)
// CHECK-SAME: vec3<f32>
// CHECK: @align(4)
// CHECK-SAME: f32

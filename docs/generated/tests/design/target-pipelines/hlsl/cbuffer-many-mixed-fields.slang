//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Stress: a cbuffer with many mixed-type fields and a matrix still emits as a single parameter-group struct followed by a cbuffer block.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: nine mixed scalar fields plus a matrix in one
// cbuffer. The lifted struct must contain every field in
// declaration order.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params {
    float a; int b; float c; uint d;
    float e; int f; float g; uint h;
    float4x4 m;
};
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = a + float(b) + c + float(d) + e + float(f) + g + float(h) + m[0][0];
}

// CHECK: struct SLANG_ParameterGroup_Params{{.*}}
// CHECK: float a_{{[0-9]+}};
// CHECK: int b_{{[0-9]+}};
// CHECK: float c_{{[0-9]+}};
// CHECK: uint d_{{[0-9]+}};
// CHECK: float4x4 m_{{[0-9]+}};
// CHECK: cbuffer Params_{{[0-9]+}} : register(b{{[0-9]+}})

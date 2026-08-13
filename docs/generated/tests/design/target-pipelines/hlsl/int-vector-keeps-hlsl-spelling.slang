//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Boundary: integer vectors keep their HLSL spelling `int<N>`, not GLSL's `ivec<N>` or WGSL's `vec<N><i32>`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Like float vectors, integer vectors use the HLSL `int<N>`
// convention. Boundary on int element type (vs float covered
// already).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int3(int(tid.x), int(tid.y), int(tid.z));
}

// CHECK: RWStructuredBuffer<int3{{.*}}> outBuf_{{[0-9]+}}

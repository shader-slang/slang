//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=e324a10260c794d77c45954ccddaf61cf66af2794e97a8289f9655330ee625d4
//META: purpose=Stress probe of the Targets table: the same dense compute kernel (scalar, vector, and matrix work; multiple buffer parameters) compiles to all 7 text-emit targets — HLSL, GLSL, SPIR-V asm, Metal, WGSL, CUDA, C++.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `target-table-all-text-emit.slang` exercises the smallest possible
// kernel against all 7 text-emit groups; this stress test does the
// same against a denser body (scalar int, 3-vector float, 4x4
// matrix, and three separate output buffers). It catches per-target
// emit regressions that surface only when multiple per-target
// legalization passes interact.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outI;
RWStructuredBuffer<float3> outV;
RWStructuredBuffer<float4x4> outM;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = int(tid.x);
    float3 v = float3(float(x), float(x) + 1.0, float(x) + 2.0);
    float4x4 m = float4x4(1, 0, 0, 0,
                          0, 1, 0, 0,
                          0, 0, 1, 0,
                          0, 0, 0, 1);
    outI[tid.x] = x * 2;
    outV[tid.x] = v + float3(0.5, 0.5, 0.5);
    outM[tid.x] = m;
}

// HLSL: numthreads(8, 1, 1)
// GLSL: local_size_x = 8
// SPIRV: OpExecutionMode %main LocalSize 8 1 1
// METAL: kernel
// WGSL: @workgroup_size(8, 1, 1)
// CUDA: __global__
// CPP: SLANG_PRELUDE_EXPORT

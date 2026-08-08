//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=a59d860b5f6a346b23bed14b56396e2c1ea41a3005067b975c01053cca1644c4
//META: purpose=A Slang `float3` is spelled with each backend's native vector syntax — `float3` on HLSL/Metal/CUDA, `vec3` on GLSL, `vec3<f32>` on WGSL, and the prelude template `Vector<float, 3>` on C++ — because `emitType` is one of the operations each subclass overrides.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-target override of type emission on the shared C-like
// base. The vector is passed to and returned from a free function so the
// type appears in a signature rather than only inside a folded expression;
// the result is summed into a buffer so nothing is eliminated. Each prefix
// asserts the signature in that backend's own spelling.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<float> outBuf;

float3 scaleIt(float3 v, float s)
{
    return v * s;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 v = float3(float(tid.x), float(tid.y), float(tid.z));
    float3 r = scaleIt(v, 2.0f);
    outBuf[0] = r.x + r.y + r.z;
}

// HLSL: float3 scaleIt_{{[0-9]+}}(float3 v_{{[0-9]+}}, float s_{{[0-9]+}})
// GLSL: vec3 scaleIt_{{[0-9]+}}(vec3 v_{{[0-9]+}}, float s_{{[0-9]+}})
// METAL: float3 scaleIt_{{[0-9]+}}(float3 v_{{[0-9]+}}, float s_{{[0-9]+}})
// WGSL: fn scaleIt_{{[0-9]+}}( v_{{[0-9]+}} : vec3<f32>,  s_{{[0-9]+}} : f32) -> vec3<f32>
// CUDA: __device__ float3  scaleIt_{{[0-9]+}}(float3  v_{{[0-9]+}}, float s_{{[0-9]+}})
// CPP: static Vector<float, 3>  scaleIt_{{[0-9]+}}(Vector<float, 3>  v_{{[0-9]+}}, float s_{{[0-9]+}})

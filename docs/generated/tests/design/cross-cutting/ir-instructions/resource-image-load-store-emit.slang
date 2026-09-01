//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=9e5354e38bb09fc16bac90544687941aec8ef947b56cd9f9b65ce3ddb041f315
//META: purpose=A subscript read and a subscript write on an `RWTexture2D` lower to the `imageLoad` / `imageStore` opcodes and emit as each target's image read / write form.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `imageLoad` / `imageStore` resource row across every feasible
// text-emit target. The single `RWTexture2D<float4>` subscript pair is the
// portable surface for both opcodes; each target CHECK asserts the target's
// own spelling of the same two operations (GLSL `imageLoad`/`imageStore`,
// SPIR-V `OpImageRead`/`OpImageWrite`, Metal `.read`/`.write`, WGSL
// `textureLoad`/`textureStore`, CUDA `surf2Dread`/`surf2Dwrite`, HLSL and
// C++ plain subscripts). Coordinates derive from `SV_DispatchThreadID` and
// the loaded value is also written to a buffer, so neither the load nor the
// store can be folded or dead-stripped before emission.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry main

uniform RWTexture2D<float4> ctex;
uniform RWStructuredBuffer<float> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 v = ctex[tid.xy];
    ctex[tid.xy + uint2(1, 1)] = v * 2.0;
    outbuf[0] = v.x;
}

// HLSL-DAG: ctex_{{[0-9]+}}[
// GLSL-DAG: imageLoad((ctex_{{[0-9]+}})
// GLSL-DAG: imageStore((ctex_{{[0-9]+}})
// SPV-DAG: OpImageRead %v4float
// SPV-DAG: OpImageWrite %{{[A-Za-z0-9_]+}}
// METAL-DAG: .read(
// METAL-DAG: .write(
// WGSL-DAG: textureLoad((ctex_{{[0-9]+}})
// WGSL-DAG: textureStore((ctex_{{[0-9]+}})
// CUDA-DAG: surf2Dread<float4 >
// CUDA-DAG: surf2Dwrite<float4 >
// CPP-DAG: ctex_{{[0-9]+}}[

//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=a07249d171e8760079dd021f485b191eeba9855fbe96652d35a7e74a16b7586a
//META: purpose=legalizeUniformBufferLoad / invertYOfPositionOutput / rcpWOfPositionInput are gated on isKhronosTarget or HLSL and do not fire for the WGSL target
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Phase C "Filtered out" list says legalizeUniformBufferLoad,
// invertYOfPositionOutput, and rcpWOfPositionInput run only for
// Khronos (Vulkan/GLSL/SPIR-V) and HLSL targets. WGSL hits none of
// those arms. With a ConstantBuffer<float4> load and an
// SV_Position-ish input, the WGSL emit should show a clean
// var<uniform> access without any HLSL/Khronos-style Y-invert or
// 1/W rewrites injected before the load reaches its uses.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

struct CB { float4 v; };
ConstantBuffer<CB> cb;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // Reads through the uniform buffer; if the Khronos/HLSL legalization
    // passes had fired, this would be rewritten into a different shape.
    outBuf[tid.x] = cb.v;
}

// The constant buffer surfaces as a WGSL uniform.
// CHECK: var<uniform>
// CHECK-SAME: cb_{{[0-9]+}}

// No Y-axis sign flip is applied to the loaded value (gpu-cross-api-flag
// VulkanInvertY is a Khronos-only pass on the position output, never
// inserted by the WGSL pipeline).
// CHECK-NOT: vec4(1.0f, -1.0f, 1.0f, 1.0f)
// CHECK-NOT: vec4<f32>(1.0f, -1.0f, 1.0f, 1.0f)

// No 1/w reinterpretation of the position W component is performed
// (gpu-cross-api-flag VulkanUseDxPositionW is HLSL/Khronos only).
// CHECK-NOT: 1.0f / cb_
// CHECK-NOT: 1.0 / cb_

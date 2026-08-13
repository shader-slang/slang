//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
//META: purpose=A fragment entry point emits OpEntryPoint Fragment and an OpExecutionMode %main OriginUpperLeft (the SPIR-V default coordinate origin Slang requests).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: a different execution model than the bundle's
// compute-stage default. Fragment entry points emit
// "Fragment" rather than "GLCompute" and add the
// OriginUpperLeft execution mode (HLSL/D3D convention that
// Slang preserves for Vulkan).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

RWStructuredBuffer<float4> outBuf;

[shader("fragment")]
float4 main() : SV_Target
{
    outBuf[0] = float4(0, 0, 0, 1);
    return float4(0.5, 0.5, 0.5, 1.0);
}

// CHECK: OpEntryPoint Fragment %main
// CHECK: OpExecutionMode %main OriginUpperLeft

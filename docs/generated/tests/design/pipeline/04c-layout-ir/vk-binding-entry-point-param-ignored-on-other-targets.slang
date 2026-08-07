//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#explicit-vkbinding-on-entry-point-parameters
//META: doc_section_digest=10008d1172c4455fc3ac51b30827a25eff107363f644dd3c90a136102fc13971
//META: purpose=On targets outside the Khronos/WGPU gate the vk::binding on an entry-point parameter is ignored and the parameter is placed positionally instead.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative half of the target gate. The same parameter that
// lands at binding 3 / set 1 on SPIR-V must be placed by the ordinary
// positional rules elsewhere: HLSL gives it `register(t0)` (not `space1`),
// Metal gives it the next positional buffer index, and CUDA has no binding
// concept at all and emits it as a plain kernel argument.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main([[vk::binding(3, 1)]] uniform StructuredBuffer<int> inBuf)
{
    outBuf[0] = inBuf[0];
}

// The requested set 1 never reaches the HLSL declaration.
// HLSL-NOT: space1
// HLSL: entryPointParams_inBuf{{[A-Za-z0-9_]*}} : register(t0)

// Metal buffer indices are positional, so the parameter follows outBuf.
// METAL: entryPointParams_inBuf{{[A-Za-z0-9_]*}} {{\[\[}}buffer(1){{\]\]}}

// CUDA has no descriptor binding surface; the parameter stays an argument.
// CUDA: __global__ void main{{[A-Za-z0-9_]*}}(StructuredBuffer<int> inBuf

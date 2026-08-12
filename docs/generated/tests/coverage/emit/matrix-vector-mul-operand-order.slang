//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies `mul(matrix, vector)` keeps an `mul(...)` call on HLSL, reverses to `vector * matrix` on GLSL, and emits `OpVectorTimesMatrix` on SPIR-V.
//META: intent=characterization
//META: covers=source/slang/slang-emit-glsl.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins how a matrix-times-vector `mul` is emitted per target under the
// default column-major layout. HLSL keeps the intrinsic `mul(M, v)`.
// GLSL has no `mul`; the c-like/GLSL emitter rewrites it as `v * M`
// (operand order reversed because GLSL `*` is column-major) — this
// reordering is the GLSL-emitter behaviour being pinned. SPIR-V emits
// the native `OpVectorTimesMatrix`. The matrix and vector are uniforms
// (not constant-folded) and the result is sunk to outputBuffer.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

uniform float4x4 mA;
uniform float4 vB;
RWStructuredBuffer<float> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4 r = mul(mA, vB) + (float)tid.x;
    outputBuffer[0] = r.x + r.y + r.z + r.w;
}

// HLSL keeps the mul() intrinsic with (matrix, vector) order.
//HLSL: mul(globalParams_0.mA_0, globalParams_0.vB_0)
// GLSL reverses to vector * matrix.
//GLSL: (globalParams_0.vB_0) * (globalParams_0.mA_0)
// SPIR-V uses the native vector-times-matrix opcode.
//SPV: OpVectorTimesMatrix
// Metal also reverses to vector * matrix (column-major).
//METAL: (globalParams_1->vB_0) * (
// WGSL also reverses to vector * matrix.
//WGSL: (globalParams_0.vB_0) * (

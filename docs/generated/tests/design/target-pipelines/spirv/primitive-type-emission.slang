//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
//META: purpose=float4 lowers to OpTypeVector %float 4 and int / uint to OpTypeInt 32 (1|0) in SPIR-V emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V's primitive type declarations: OpTypeFloat 32 for
// float, OpTypeInt 32 1 for int, OpTypeInt 32 0 for uint, and
// OpTypeVector %float 4 for float4. All four appear in any
// non-trivial Slang compute shader.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float4(float(tid.x), 1.0, 2.0, 3.0);
}

// CHECK-DAG: %uint = OpTypeInt 32 0
// CHECK-DAG: %float = OpTypeFloat 32
// CHECK-DAG: %v4float = OpTypeVector %float 4

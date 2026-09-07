//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
//META: purpose=The uint32 MAX literal (0xFFFFFFFFu) emits as OpConstant %uint 4294967295 (decimal in spirv-asm).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: the upper edge of the uint32 range. The SPIR-V
// disassembler prints unsigned integer constants in decimal,
// so 0xFFFFFFFFu surfaces as 4294967295.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 0xFFFFFFFFu;
}

// CHECK: %uint_4294967295 = OpConstant %uint 4294967295

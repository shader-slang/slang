//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
//META: purpose=A compute entry point's function body is wrapped in OpFunction %void None %funcType ... OpReturn OpFunctionEnd.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Every SPIR-V function declaration follows the same
// OpFunction ... OpReturn ... OpFunctionEnd skeleton; an
// entry-point function with void return and no non-builtin
// parameters fits the canonical form.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
        return;
    outBuf[tid.x] = 1;
}

// CHECK: %main = OpFunction %void None
// CHECK: OpReturn
// CHECK: OpFunctionEnd

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#eliminatephis-with-spir-v-specific-options
//META: doc_section_digest=b36f8fb17f73698f8db16cbc260b4d3068995c13f2e365b125cbcf5423d67537
//META: purpose=eliminatePhis on SPIR-V uses useRegisterAllocation=true so an if/else-merged value becomes a Function-storage OpVariable populated by OpStore in each branch (no surviving OpPhi).
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// On SPIR-V, eliminatePhis runs with useRegisterAllocation=true.
// The register-allocator coalesces SSA values into a named local
// in Function storage class; each branch performs an OpStore into
// that variable. No OpPhi survives.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x;
    if (tid.x > 0)
        x = 10;
    else
        x = 20;
    outBuf[tid.x] = x;
}

// CHECK: %x = OpVariable %_ptr_Function_int Function
// CHECK-DAG: OpStore %x %int_10
// CHECK-DAG: OpStore %x %int_20

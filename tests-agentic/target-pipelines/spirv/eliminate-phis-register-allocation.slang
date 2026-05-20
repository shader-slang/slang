//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#eliminatephis-with-spir-v-specific-options
//META: doc_section_digest=fe05b23b1c0443d967eca4c904387718a54e3d95f22f2d7525a5b76c28360580
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

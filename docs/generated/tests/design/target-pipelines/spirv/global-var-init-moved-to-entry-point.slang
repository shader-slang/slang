//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=moveGlobalVarInitializationToEntryPoints turns a module-scope static initializer into a Function-storage variable stored at the top of the entry point.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the SPIR-V-only Phase C pass moveGlobalVarInitializationToEntryPoints:
// SPIR-V has no module-scope initializer for a mutable global, so the
// initialization has to become an explicit store inside the entry point. The
// static is mutated across a data-dependent loop so it cannot be folded into a
// constant; CHECK asserts the variable is a Function-storage OpVariable and
// that its initial value 7 is written with an OpStore inside main.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;
StructuredBuffer<int> inBuf;

static int g = 7;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int n = inBuf[tid.x];
    for (int i = 0; i < n; i++)
        g = g + i;
    outBuf[tid.x] = g;
}

// CHECK: OpFunction
// CHECK: OpVariable %_ptr_Function_int Function
// CHECK: OpStore %{{[a-zA-Z_0-9]+}} %int_7

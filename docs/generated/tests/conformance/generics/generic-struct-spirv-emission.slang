//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic compute shader specialized with a value parameter emits valid SPIR-V that can be checked; the entry point and storage buffer are present in the assembly.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// After specialization of a generic struct with a value param, the SPIR-V
// must include the entry point and proper StorageBuffer bindings.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

struct Sized<let N : uint>
{
    int data[N];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    Sized<4> s;
    s.data[0] = 1;
    s.data[1] = 2;
    s.data[2] = 3;
    s.data[3] = 4;
    outputBuffer[0] = s.data[0] + s.data[3];
}
// CHECK: OpEntryPoint GLCompute %computeMain
// CHECK: StorageBuffer

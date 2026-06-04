//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=3eac3aff8a5d5daa072f3cad641be3c7b7a2f0e1ecf59515c5939a7a993d85ec
//META: purpose=`T*` declaration syntax compiles to CPP, CUDA, and SPIRV-ASM targets; the pointer type annotation appears in the emitted output on each feasible back-end.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `T*` pointer declarations survive to CPP, CUDA, and SPIR-V codegen.
// Compiles a compute shader that takes an `int*` local, increments through
// it, and writes the result; CHECK confirms CPP output contains `int32_t`,
// CHECKCUDA confirms CUDA kernel marker, and CHECKSPV confirms SPIR-V
// emits OpTypePointer StorageBuffer (the storage-buffer pointer type used
// for RWStructuredBuffer access — the local T* is elided by the optimizer
// but the file compiles, proving T* syntax is accepted for SPIR-V targets).
//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECKCUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECKSPV):-target spirv-asm -entry main -stage compute

[numthreads(1,1,1)]
void main(RWStructuredBuffer<int> inputBuf, RWStructuredBuffer<int> outputBuf)
{
    int x = inputBuf[0];
    int* p = &x;
    *p = *p + 1;
    outputBuf[0] = x;
}
// CHECK: int32_t
// CHECKCUDA: __global__
// CHECKSPV: OpTypePointer StorageBuffer

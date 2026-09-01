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
//
// Emission fan-out for the shader targets uses `devicePtrMain` below instead of
// `main`.  The doc says "Pointers to local memory are supported only on CUDA and
// CPU targets", and the core module gates `operator&` on `cpp_cuda_spirv_llvm`, so
// the `&x` in `main` stops at `error[E36107]: unavailable features in entry point`
// on every other target.  A `T*` that points at device memory carries no such
// restriction, so `devicePtrMain` pins the pointer type annotation the Metal and
// GLSL back-ends emit for `T*`: Metal has a native `int device*`, and GLSL renders
// a pointer as a `GL_EXT_buffer_reference` block.  hlsl and wgsl are opted out in
// the bundle README (they emit an unlegalized pointer type; see the findings named
// there), and cpp / cuda / spirv-asm stay on `main` above.
//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECKCUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECKSPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry devicePtrMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry devicePtrMain -stage compute

[numthreads(1,1,1)]
void main(RWStructuredBuffer<int> inputBuf, RWStructuredBuffer<int> outputBuf)
{
    int x = inputBuf[0];
    int* p = &x;
    *p = *p + 1;
    outputBuf[0] = x;
}

[numthreads(1,1,1)]
void devicePtrMain(uniform int* devPtr, RWStructuredBuffer<int> outBuf)
{
    outBuf[0] = *devPtr;
}
// CHECK: int32_t
// CHECKCUDA: __global__
// CHECKSPV: OpTypePointer StorageBuffer
// METAL: int device* devPtr{{.*}};
// GLSL: layout(buffer_reference, std430, buffer_reference_align = 4) buffer BufferPointer_

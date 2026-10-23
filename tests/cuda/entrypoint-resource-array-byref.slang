// Verifies the CUDA by-reference transform for runtime-indexed resource-array entry-point uniforms.
//
// Slang passes entry-point parameters by value, so a fixed-size resource array uniform
// (`RWStructuredBuffer<float> tensors[8]`) becomes a by-value `FixedArray<...>` kernel argument in
// the CUDA `.param` space. ptxas cannot dynamically index `.param`, so `tensors[indices[tid]]`
// (a runtime index) lowers to a serial `ld.param` chain. `EntryPointInParamToBorrowContext` now
// passes such a uniform by reference (a pointer), so the index becomes an ordinary
// dynamically-addressable load through the pointer.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none -stage compute -entry main

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    uint tid: SV_DispatchThreadID,
    RWStructuredBuffer<float> tensors[8],
    StructuredBuffer<uint> indices)
{
    tensors[indices[tid]][tid] = 1.0f;
}

// The runtime-indexed resource array `tensors` is a pointer parameter (by reference), and is
// indexed through the pointer rather than copied by value into the kernel `.param` bank.
// CHECK: void main{{.*}}* tensors
// CHECK: (*tensors{{.*}})[

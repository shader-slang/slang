//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// A `ParameterBlock` (or `ConstantBuffer`) whose element is a fixed-size array used
// to crash the C++/CUDA emitter twice over: `calcTypeName` had no case for array
// types (E99999), and the `GetElementPtr` special-casing dereferenced the base as an
// `IRPtrTypeBase` without checking, crashing on a pointer-*like* (parameter group)
// base. The block must emit as a `FixedArray<T, N>*` parameter indexed through the
// generic dereference path.

// CHECK: __global__ void computeMain(FixedArray<float, 4>* arr_0, RWStructuredBuffer<float> output_0)
// CHECK: (*arr_0)[

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform ParameterBlock<float[4]> arr,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = arr[tid.x];
}

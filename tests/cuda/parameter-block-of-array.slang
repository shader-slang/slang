//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPPCHECK): -target cpp -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CBCHECK): -target cuda -entry cbMain -stage compute -line-directive-mode none -DUSE_CB

// A `ParameterBlock` (or `ConstantBuffer`) whose element is a fixed-size array used
// to crash the C++/CUDA emitter twice over: `calcTypeName` had no case for array
// types (E99999), and the `GetElementPtr` special-casing dereferenced the base as an
// `IRPtrTypeBase` without checking, crashing on a pointer-*like* (parameter group)
// base. The block must emit as a `FixedArray<T, N>*` parameter indexed through the
// generic dereference path.

// The fix lives in CPPSourceEmitter, shared by the CUDA and CPP targets, and the
// same emit case handles ConstantBuffer; pin all of those combinations.
// CHECK: __global__ void computeMain(FixedArray<float, 4>* arr_0, RWStructuredBuffer<float> output_0)
// CHECK: (*arr_0)[
// On CPP targets entry-point uniforms are collected into a params struct passed
// via void*; the block still renders as a FixedArray pointer field there.
// CPPCHECK: FixedArray<float, 4>* arr_0;
// CPPCHECK: ->arr_0)[
// CBCHECK: __global__ void cbMain(FixedArray<float, 4>* arr_0, RWStructuredBuffer<float> output_0)
// CBCHECK: (*arr_0)[

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform ParameterBlock<float[4]> arr,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = arr[tid.x];
}

#ifdef USE_CB
[shader("compute")]
[numthreads(4, 1, 1)]
void cbMain(
    uint3 tid: SV_DispatchThreadID,
    uniform ConstantBuffer<float[4]> arr,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = arr[tid.x];
}
#endif

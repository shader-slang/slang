//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none -cuda-entry-point-params-by-value

// Same as kernel-param-dynamic-index-local-copy.slang, but with the array nested
// one struct level down and its elements being pointer-backed structs — the shape
// SlangPy's functional API generates for a tensor array. The dynamic-index
// legalization must root the access chain through the parameter's local copy
// while leaving statically-addressed accesses to the same parameter (the
// `indices` field here) on the parameter itself.
//
// By default this parameter shape is passed by reference instead (see
// entry-point-uniform-descriptor-array-byref.slang); the local-copy floor tested
// here is what covers it under the `-cuda-entry-point-params-by-value` legacy ABI
// (and kernels excluded from the by-reference gate, e.g. [CudaKernel] functions).

struct MyTensor
{
    float* data;
    uint stride;
}

struct CallData
{
    MyTensor tensors[32];
    StructuredBuffer<uint> indices;
}

// CHECK: __global__ void computeMain(CallData_0 call_0)
// CHECK: CallData_0 [[LOCAL:_S[0-9]+]] = call_0;
// The statically-addressed field stays on the parameter (fast `.param` path).
// CHECK: __ldg((&(call_0.indices_0)
// The runtime-indexed array goes through the local copy.
// CHECK-NOT: call_0.tensors_0[
// CHECK: [[LOCAL]]{{.*}}tensors_0[

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform CallData call)
{
    *(call.tensors[call.indices[tid.x]].data + tid.x) = 1.0f;
}

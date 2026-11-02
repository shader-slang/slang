// A body-less `__extern_cpp` function that is not an entry point binds to a C/C++ definition
// outside the Slang module, so a kernel may call it without an unresolved-symbol error. The
// entry-point counterpart is rejected; see extern-cpp-entry-point-without-body.slang.

//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute

// CPP: float externHelper(float {{.*}});
// CUDA: __device__ float externHelper(float {{.*}});
__extern_cpp float externHelper(float x);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform float* p, uint3 tid : SV_DispatchThreadID)
{
    // CPP: = externHelper(
    // CUDA: = externHelper(
    p[tid.x] = externHelper(p[tid.x]);
}

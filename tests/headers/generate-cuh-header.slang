//TEST:SIMPLE(filecheck=CHECK): -target cuh -line-directive-mode none

//CHECK: #pragma once
//CHECK: #include "{{.*}}slang-cuda-prelude.h"

// see "sharedMemory" below
//CHECK: __device__ __shared__ FixedArray<float, 4>  sharedMemory;

//CHECK: struct Struct
//CHECK: {
//CHECK:     float x;
//CHECK:     RWStructuredBuffer<float> vec;
//CHECK:     RWStructuredBuffer<float> result;
//CHECK: };
__extern_cpp struct Struct
{
    __extern_cpp float x;
    __extern_cpp RWStructuredBuffer<float> vec;
    __extern_cpp RWStructuredBuffer<float> result;
};

// for some reason generated before struct
__extern_cpp groupshared float sharedMemory[4];

//CHECK: __global__ void kernel(Struct {{[[:alnum:]_]+}})
[CudaKernel]
void kernel(Struct s)
{
    uint3 globalIdx = uint3(cudaBlockIdx()) * uint3(cudaBlockDim()) + uint3(cudaThreadIdx());
    sharedMemory[globalIdx.x] = s.vec[globalIdx.x] + s.x;
    InterlockedAdd(s.result[0], sharedMemory[globalIdx.x]);
}

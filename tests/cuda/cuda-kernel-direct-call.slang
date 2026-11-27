// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none
//TEST:COMPARE_COMPUTE(targets=cuda, filecheck-buffer=CHECK):-output-using-type -Xslang -O0
//TEST:COMPARE_COMPUTE(targets=cuda, filecheck-buffer=CHECK):-output-using-type -Xslang -O3

// A kernel remains externally launchable, but direct calls must use an ordinary device clone.
// Prevent inlining so the CUDA source check and runtime tests exercise that distinction.
// CUDA-DAG: __global__ void fillIDs
// CUDA-DAG: static __device__ __noinline__ void fillIDs
[CUDAKernel]
[noinline]
void fillIDs(uint count, uint bias, uint offset, RWStructuredBuffer<uint> output)
{
    uint index = cudaBlockIdx().x * cudaBlockDim().x + cudaThreadIdx().x;
    if (index < count)
        output[offset + index] = index + bias;
}

[noinline]
void forwardFill(RWStructuredBuffer<uint> output)
{
    fillIDs(3, 11, 4, output);
}

//TEST_INPUT:ubuffer(data=[99 99 99 99 99 99 99 99], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain()
{
    fillIDs(3, 7, 0, outputBuffer);
    forwardFill(outputBuffer);
}

// CHECK: 7
// CHECK: 8
// CHECK: 9
// CHECK: 99
// CHECK: 11
// CHECK: 12
// CHECK: 13
// CHECK: 99

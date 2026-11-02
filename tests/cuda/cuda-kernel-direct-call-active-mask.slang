// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -O0 -validate-ir
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -O3

// Only lanes 0 and 2 call `writeActiveLaneCount`, so each must observe two active lanes.
// We keep the kernel body separate to verify that its ordinary device clone receives the caller's
// active mask instead of computing the mask used for a fresh kernel launch.
// CUDA-DAG: __global__ void writeActiveLaneCount{{[A-Za-z0-9_]*}}(uint {{[A-Za-z0-9_]+}}, RWStructuredBuffer<uint> {{[A-Za-z0-9_]+}})
// CUDA-DAG: static __device__ __noinline__ void [[DEVICE_KERNEL:writeActiveLaneCount_[0-9]+]](uint {{[A-Za-z0-9_]+}}, RWStructuredBuffer<uint> {{[A-Za-z0-9_]+}}, uint {{[A-Za-z0-9_]+}})
[CUDAKernel]
[noinline]
void writeActiveLaneCount(uint index, RWStructuredBuffer<uint> output)
{
    output[index] = WaveActiveSum(1u);
}

//TEST_INPUT:ubuffer(data=[99 99 99 99], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// We call the kernel from divergent control flow without any mutable resource global that would
// otherwise select a separate pipeline path. The odd lanes keep their initial sentinel values.
[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    if ((index & 1u) == 0)
        writeActiveLaneCount(index, outputBuffer);
}

// CUDA: bool [[EVEN_LANE:[A-Za-z0-9_]+]] = {{.*}} == 0U;
// CUDA: uint [[CALL_MASK:[A-Za-z0-9_]+]] = __ballot_sync({{[A-Za-z0-9_]+}}, [[EVEN_LANE]]);
// CUDA: [[DEVICE_KERNEL]]({{.*}}, {{.*}}, [[CALL_MASK]]);
// CHECK: {{^}}2{{$}}
// CHECK: {{^}}99{{$}}
// CHECK: {{^}}2{{$}}
// CHECK: {{^}}99{{$}}

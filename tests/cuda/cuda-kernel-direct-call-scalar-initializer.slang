// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -O0 -validate-ir
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -O3

// A non-inlined initializer keeps the mutable scalar global live at `-O0`.
[noinline]
uint getInitialCounter()
{
    return 3;
}

static uint counter = getInitialCounter();

// We increment the same counter in an ordinary helper and in direct calls to the kernel.
// We initialize the counter separately for each launched thread; direct calls use the caller's
// existing counter.
[noinline]
uint getNextCounter()
{
    return counter++;
}

// The launch signature has only its declared parameters. The device clone additionally receives
// the `KernelContext` containing the mutable counter.
// CUDA-DAG: __global__ void writeNextCounter{{[A-Za-z0-9_]*}}(RWStructuredBuffer<uint> {{[A-Za-z0-9_]+}}, uint {{[A-Za-z0-9_]+}})
// CUDA-DAG: static __device__ __noinline__ void [[DEVICE_KERNEL:writeNextCounter_[0-9]+]](RWStructuredBuffer<uint> {{[A-Za-z0-9_]+}}, uint {{[A-Za-z0-9_]+}}, KernelContext{{[A-Za-z0-9_]*}} * {{[A-Za-z0-9_]+}})
[CUDAKernel]
[noinline]
void writeNextCounter(RWStructuredBuffer<uint> output, uint index)
{
    output[index] = getNextCounter();
}

//TEST_INPUT:ubuffer(data=[99 99 99], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// The helper reads 3 before the calls; the two device calls must continue with 4 and 5.
[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    outputBuffer[0] = getNextCounter();
    writeNextCounter(outputBuffer, 1);
    writeNextCounter(outputBuffer, 2);
}

// Each executing thread creates and initializes its own context; its direct calls reuse it.
// CUDA: KernelContext{{[A-Za-z0-9_]*}} [[LAUNCH_CONTEXT:[A-Za-z0-9_]+]];
// CUDA: (&[[LAUNCH_CONTEXT]])->counter_{{[0-9]+}} = [[INITIALIZER:[A-Za-z0-9_]+]]();
// CUDA-LABEL: extern "C" __global__ void computeMain()
// CUDA: KernelContext{{[A-Za-z0-9_]*}} [[CALLER_CONTEXT:[A-Za-z0-9_]+]];
// CUDA: (&[[CALLER_CONTEXT]])->counter_{{[0-9]+}} = [[INITIALIZER]]();
// CUDA: [[DEVICE_KERNEL]]({{.*}}, 1U, &[[CALLER_CONTEXT]]);
// CUDA: [[DEVICE_KERNEL]]({{.*}}, 2U, &[[CALLER_CONTEXT]]);
// CHECK: {{^}}3{{$}}
// CHECK: {{^}}4{{$}}
// CHECK: {{^}}5{{$}}

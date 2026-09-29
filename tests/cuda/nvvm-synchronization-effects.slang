//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -compute-dispatch 2,1,1 -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -compute-dispatch 2,1,1 -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -compute-dispatch 2,1,1 -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -
//TEST_INPUT: set iterationBuffer = ubuffer(data=[2], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)

StructuredBuffer<uint> iterationBuffer;
RWStructuredBuffer<uint> outputBuffer;
groupshared uint values[64];

[noinline]
void synchronize()
{
    __target_switch
    {
    case nvvm: __intrinsic_asm "llvm.nvvm.barrier0";
    case cuda: __intrinsic_asm "__syncthreads()";
    }
}

[noinline]
void orderGroupMemory()
{
    __target_switch
    {
    case nvvm: __intrinsic_asm "llvm.nvvm.membar.cta";
    case cuda: __intrinsic_asm "__threadfence_block()";
    }
}

[noinline]
void orderDeviceMemory()
{
    __target_switch
    {
    case nvvm: __intrinsic_asm "llvm.nvvm.membar.gl";
    case cuda: __intrinsic_asm "__threadfence()";
    }
}

[noinline]
void storeUpperWarp(uint index, uint neighbor)
{
    values[index] = neighbor + 3;
}

[noinline]
void storeLowerWarp(uint index, uint neighbor)
{
    values[index] = neighbor + 1;
}

[numthreads(64, 1, 1)]
void computeMain(uint index : SV_GroupIndex)
{
    values[index] = index;
    synchronize();
    // Every lane reaches every barrier. The warp-index branch is invariant across the loop.
    // Moving either barrier inside that branch would break communication between lanes paired
    // across warps.
    for (uint iteration = 0; iteration < iterationBuffer[0]; ++iteration)
    {
        uint neighbor = values[index ^ 32];
        synchronize();
        if ((index & 32) != 0)
            storeUpperWarp(index, neighbor);
        else
            storeLowerWarp(index, neighbor);
        orderGroupMemory();
        synchronize();
        orderDeviceMemory();
    }
    uint result = values[index];
    InterlockedAdd(outputBuffer[0], 1);
    InterlockedAdd(outputBuffer[1], result == index + 4 ? 0 : 1);
    InterlockedAdd(outputBuffer[2], result);
}

// Each pair exchanges values twice and adds 1 + 3. Across two independent groups,
// the host oracle is 128 invocations, zero errors, and 2 * (sum(0..63) + 64 * 4).
// CHECK: 128
// CHECK-NEXT: 0
// CHECK-NEXT: 4544

// PTX-DAG: bar.sync 0
// PTX-DAG: membar.cta
// PTX-DAG: membar.gl

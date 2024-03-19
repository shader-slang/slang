//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 460

// CHECK_GLSL: void main(
// CHECK_SPV: OpEntryPoint

//TEST_INPUT:ubuffer(data=[2 2 2 2], stride=4):out,name=raceCondition
buffer MyBlockName1
{
    uint data[];
} raceCondition;

shared uint raceConditionShared;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

bool testMemoryBarrier()
{
    raceCondition.data[0] = gl_GlobalInvocationID.x;
    memoryBarrier();
    raceCondition.data[0] = 1;
    memoryBarrier();
    return true
        && raceCondition.data[0] == 1
        ;
}
bool testMemoryBarrierAtomicCounter()
{
    atomicExchange(raceCondition.data[1], 2);
    memoryBarrierAtomicCounter();
    atomicExchange(raceCondition.data[1], 1);
    memoryBarrierAtomicCounter();
    return true
        && raceCondition.data[1] == 1
        ;
}
bool testMemoryBarrierBuffer()
{
    raceCondition.data[2] = gl_GlobalInvocationID.x;
    memoryBarrierBuffer();
    raceCondition.data[2] = 1;
    memoryBarrierBuffer();
    return true
        && raceCondition.data[2] == 1
        ;
}
bool testMemoryBarrierShared()
{
    raceConditionShared = 2;
    memoryBarrierShared();
    raceConditionShared = 1;
    memoryBarrierShared();
    return true
        && raceConditionShared == 1
        ;
}
bool testMemoryBarrierImage()
{
    // image buffers are currently unimplemented
    return true
        ;
}
bool testGroupMemoryBarrier()
{
    raceCondition.data[3] = gl_GlobalInvocationID.x;
    memoryBarrierImage();
    raceCondition.data[3] = 1;
    memoryBarrierImage();
    return true
        && raceCondition.data[3] == 1
        ;
}
bool testMemoryBarriers()
{
    return true
        && testMemoryBarrier()
        && testMemoryBarrierAtomicCounter()
        && testMemoryBarrierBuffer()
        && testMemoryBarrierShared()
        && testMemoryBarrierImage()
        && testGroupMemoryBarrier()
        ;
}

void computeMain()
{
    outputBuffer.data[0] = true
        && testMemoryBarriers()
        ;
    // BUF: 1
}

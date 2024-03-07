//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl -DTARGET_GLSL
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly -DTARGET_SPIRV
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    uint data[2];
} outputBuffer;


//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):name=one
layout(binding = 1, offset = 12) uniform atomic_uint one;
bool testSetterAndGetter()
{
    return true
// CHECK_GLSL-N: atomicCounterExchange
// CHECK_SPV-N: OpAtomicExchange
        && atomicCounterExchange(one, 1) == 0
// CHECK_GLSL-N: one_0.data_0[3]
// no idea how to check the spirv reliabley...
        && atomicCounter(one) == 1
        && atomicCounterExchange(one, 5) == 1
        && atomicCounter(one) == 5
        ;
}

bool counterAsParam(atomic_uint param)
{
    return true
        && atomicCounterExchange(param, 5) != 100
// CHECK_GLSL-N: atomicCounterIncrement(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicIAdd
        && atomicCounterIncrement(param) == 5
// CHECK_GLSL-N: one_0.data_0[3]
        && atomicCounter(param) == 6
        ;
}

bool testAtomicUint()
{
    // ensure the code emits for `one` index into [3] for 12/4
    // CHECK_GLSL-DAG-N: {{.*}}_data_0[3]{{.*}}
    return true

// CHECK_GLSL-N: atomicCounterIncrement(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicIAdd
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterIncrement(one) == 5
        && atomicCounter(one) == 6

// CHECK_GLSL-N: atomicCounterDecrement(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicISub
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterDecrement(one) == 4

// CHECK_GLSL-N: atomicCounterAdd(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicIAdd
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterAdd(one, 1) == 5
        && atomicCounter(one) == 6

// CHECK_GLSL-N: atomicCounterDecrement(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicISub
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterSubtract(one, 1) == 5
        && atomicCounter(one) == 4

// CHECK_GLSL-N: atomicCounterMin(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicUMin
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterMin(one, 1) == 5
        && atomicCounter(one) == 1

// CHECK_GLSL-N: atomicCounterMax(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicUMax
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterMax(one, 1) == 5 
        && atomicCounter(one) == 5

// CHECK_GLSL-N: atomicCounterAnd(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicAnd
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterAnd(one, 2) == 5
        && atomicCounter(one) == 0

// CHECK_GLSL-N: atomicCounterOr(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicOr
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterOr(one, 8) == 5
        && atomicCounter(one) == 13

// CHECK_GLSL-N: atomicCounterXor(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicXor
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterXor(one, 4) == 5
        && atomicCounter(one) == 1

// CHECK_GLSL-N: atomicCounterCompSwap(one_0._data_0[3]
// CHECK_SPV-N: OpAtomicCompareExchange
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterCompSwap(one, 5, 3) == 5 
        && atomicCounter(one) == 3

// CHECK_GLSL-N: atomicCounterCompSwap(one_0._data_0[4]
// CHECK_SPV-N: OpAtomicCompareExchange
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterCompSwap(one, 5, 3) == 5 
        && atomicCounter(one) == 3

        && counterAsParam(one);
        ;
}

void computeMain()
{
    outputBuffer.data[0] = true
            && testSetterAndGetter()
            ;
    outputBuffer.data[1] = true
            && testAtomicUint()
            ;
    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // BUF: 1
    // BUF-NEXT: 1
}

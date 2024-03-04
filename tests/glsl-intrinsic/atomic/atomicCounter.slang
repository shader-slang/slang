//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl -DTARGET_GLSL
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly -DTARGET_SPIRV

// currently tests do not handle atomic_uint->storage-buffer too well (mapping to binding location)
//DISABLE_TEST:TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//DISABLE_TEST:TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out
layout(binding = 1, offset = 12) uniform atomic_uint one;
layout(binding = 1) uniform atomic_uint two;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    uint data[2];
} outputBuffer;

bool testSetterAndGetter()
{
    return true
// CHECK_GLSL-DAG-N: atomicCounterExchange
// CHECK_SPV-DAG-N: OpAtomicExchange
        && atomicCounterExchange(one, 1) == 0
// CHECK_GLSL-DAG-N: one_0.data_0[3]
// no idea how to check spirv...
        && atomicCounter(one) == 1
        && atomicCounterExchange(one, 5) == 1
        && atomicCounter(one) == 5
        ;
}

bool counterAsParam(atomic_uint param)
{
// these tests are not a -DAG test since order is important to ensure 
// code is emitted correctly (this is a function call most importantly) 
    return true
        && atomicCounterExchange(param, 5) != 100
// CHECK_GLSL-N: atomicCounterIncrement(one0._data_0[3]
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
    // ensure the code emits for `two` index into [4] for 16/4
    // CHECK_GLSL-DAG-N: {{.*}}_data_0[4]{{.*}}
    return true

// CHECK_GLSL-DAG-N: atomicCounterIncrement(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicIAdd
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterIncrement(one) == 5
        && atomicCounter(one) == 6

// CHECK_GLSL-DAG-N: atomicCounterDecrement(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicISub
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterDecrement(one) == 4

// CHECK_GLSL-DAG-N: atomicCounterAdd(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicIAdd
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterAdd(one, 1) == 5
        && atomicCounter(one) == 6

// CHECK_GLSL-DAG-N: atomicCounterDecrement(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicISub
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterSubtract(one, 1) == 5
        && atomicCounter(one) == 4

// CHECK_GLSL-DAG-N: atomicCounterMin(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicUMin
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterMin(one, 1) == 5
        && atomicCounter(one) == 1

// CHECK_GLSL-DAG-N: atomicCounterMax(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicUMax
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterMax(one, 1) == 5 
        && atomicCounter(one) == 5

// CHECK_GLSL-DAG-N: atomicCounterAnd(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicAnd
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterAnd(one, 2) == 5
        && atomicCounter(one) == 0

// CHECK_GLSL-DAG-N: atomicCounterOr(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicOr
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterOr(one, 8) == 5
        && atomicCounter(one) == 13

// CHECK_GLSL-DAG-N: atomicCounterXor(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicXor
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterXor(one, 4) == 5
        && atomicCounter(one) == 1

// CHECK_GLSL-DAG-N: atomicCounterCompSwap(one0._data_0[3]
// CHECK_SPV-DAG-N: OpAtomicCompareExchange
        && atomicCounterExchange(one, 5) != 100
        && atomicCounterCompSwap(one, 5, 3) == 5 
        && atomicCounter(one) == 3

// CHECK_GLSL-DAG-N: atomicCounterCompSwap(one0._data_0[4]
// CHECK_SPV-DAG-N: OpAtomicCompareExchange
           && atomicCounterExchange(two, 5) != 100
        && atomicCounterCompSwap(two, 5, 3) == 5 
        && atomicCounter(two) == 3

           && counterAsParam(one);
        ;
}

void computeMain()
{
    // to test an atomic we have the following pattern
    // **set val (exchange)**, operand, **test val (atomicCounter)**
    // to ensure the test works accordingly we must make a 
    // seperate test `testSetterAndGetter` for signaling when 
    // testAtomicUint will be always wrong
    // this is to remove dependency on other ops when testing
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

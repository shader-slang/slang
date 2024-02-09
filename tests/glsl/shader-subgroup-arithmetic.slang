//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv
//TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl
//TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda
//TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

precision highp float;
precision highp int;

//TEST_INPUT:ubuffer(data=[9 0 0 0 0], stride=4):name=inputBuffer
buffer MyBlockName
{
    uint data[];
} inputBuffer;

// TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

//__generic<N : __BuiltinArithmeticType, M : __BuiltinIntegerType> 
//void testBasedOnType(uint group, N test, M test2) {
    
//    uint index = 21 * group;
//    uint v[7] = uint[7](0, 0, 0, 0, 0, 0, 0); // done to issolate unimplemented functions

//    outputBuffer.data[index + 0] = subgroupAdd<N>(test) == 32;
//    outputBuffer.data[index + 1] = subgroupMul<N>(test) == 1;
//    outputBuffer.data[index + 2] = subgroupMin<N>(test) == test;
//    outputBuffer.data[index + 3] = subgroupMax<N>(test) == test;
    
//    outputBuffer.data[index + 4] = subgroupAnd<M>(test2) == 0;
//    outputBuffer.data[index + 5] = subgroupOr<M>(test2) == 63;
//    outputBuffer.data[index + 6] = subgroupXor<M>(test2) == 0;


//    // compare with goal of true with invocation id 2, not 0
//    v[0] = subgroupExclusiveAdd<N>(test) == 1;
//    v[1] = subgroupExclusiveMul<N>(test) == 1;
//    v[2] = subgroupExclusiveMin<N>(test) == 1;
//    v[3] = subgroupExclusiveMax<N>(test) == 1;

//    v[4] = subgroupExclusiveAnd<M>(test2) == 0;
//    v[5] = subgroupExclusiveOr<M>(test2) == 3;
//    v[6] = subgroupExclusiveXor<M>(test2) == 0;

    
//    if (gl_GlobalInvocationID.x == 2) {
//        outputBuffer.data[index + 7] = v[0];
//        outputBuffer.data[index + 8] = v[1];
//        outputBuffer.data[index + 9] = v[2];
//        outputBuffer.data[index + 10] = v[3];
//        outputBuffer.data[index + 11] = v[4];
//        outputBuffer.data[index + 12] = v[5];
//        outputBuffer.data[index + 13] = v[6];
//    }

//    outputBuffer.data[index + 14] = subgroupInclusiveAdd<N>(test) == 1;
//    outputBuffer.data[index + 15] = subgroupInclusiveMul<N>(test) == 1;
//    outputBuffer.data[index + 16] = subgroupInclusiveMin<N>(test) == 1;
//    outputBuffer.data[index + 17] = subgroupInclusiveMax<N>(test) == 1;

//    outputBuffer.data[index + 18] = subgroupInclusiveAnd<M>(test) == 1;
//    outputBuffer.data[index + 19] = subgroupInclusiveOr<M>(test) == 1;
//    outputBuffer.data[index + 20] = subgroupInclusiveXor<M>(test) == 1;
    
//}
void computeMain()
{
    // note: missing cuda and cpp implementation for system variables; test #3 and #4 will fail
    // note: all implementations cause a silent shader crash 
    // TODO: make more robust of a test than simple call verification

    // testBasedOnType(0, (float)1, (int)1);
    // testBasedOnType(1, (int)1, (uint)1);
    // testBasedOnType(2, (uint)1, (int)1);
    // testBasedOnType(3, (double)1, (uint)1);
    // testBasedOnType(4, (half)1, (int)1);

    uint group = 0;
    uint index = 21 * group;
    uint v[7] = uint[7](0, 0, 0, 0, 0, 0, 0); // done to issolate unimplemented functions

    float test = 1.0f;
    int test2 = 1;

    outputBuffer.data[index + 0] = subgroupAdd(test) == 32;
    outputBuffer.data[index + 1] = subgroupMul(test) == 1;
    outputBuffer.data[index + 2] = subgroupMin(test) == test;
    outputBuffer.data[index + 3] = subgroupMax(test) == test;

    outputBuffer.data[index + 4] = subgroupAnd(test2) == 0;
    outputBuffer.data[index + 5] = subgroupOr(test2) == 63;
    outputBuffer.data[index + 6] = subgroupXor(test2) == 0;

    // compare with goal of true with invocation id 2, not 0
    v[0] = subgroupExclusiveAdd(test) == 1;
    v[1] = subgroupExclusiveMul(test) == 1;
    v[2] = subgroupExclusiveMin(test) == 1;
    v[3] = subgroupExclusiveMax(test) == 1;

    v[4] = subgroupExclusiveAnd(test2) == 0;
    v[5] = subgroupExclusiveOr(test2) == 3;
    v[6] = subgroupExclusiveXor(test2) == 0;

    if (gl_GlobalInvocationID.x == 2) {
        outputBuffer.data[index + 7] = v[0];
        outputBuffer.data[index + 8] = v[1];
        outputBuffer.data[index + 9] = v[2];
        outputBuffer.data[index + 10] = v[3];
        outputBuffer.data[index + 11] = v[4];
        outputBuffer.data[index + 12] = v[5];
        outputBuffer.data[index + 13] = v[6];
    }

    outputBuffer.data[index + 14] = subgroupInclusiveAdd(test) == 1;
    outputBuffer.data[index + 15] = subgroupInclusiveMul(test) == 1;
    outputBuffer.data[index + 16] = subgroupInclusiveMin(test) == 1;
    outputBuffer.data[index + 17] = subgroupInclusiveMax(test) == 1;

    outputBuffer.data[index + 18] = subgroupInclusiveAnd(test2) == 1;
    outputBuffer.data[index + 19] = subgroupInclusiveOr(test2) == 1;
    outputBuffer.data[index + 20] = subgroupInclusiveXor(test2) == 1;

    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
}

//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv

// not testing hlsl due to missing impl
// TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl
// not testing cuda due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda 
// not testing cpp due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

// not testing hlsl due to missing impl
// TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl
// not testing cuda due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda 
// not testing cpp due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

//#define TEST_when_doubles_dont_crash

precision highp float;
precision highp int;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2 
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

__generic<T : __BuiltinLogicalType>
bool test1ShuffleX() {
    return true
        && subgroupShuffle(T(1), 1) == T(1)
        && subgroupShuffleXor(T(1), 1) == T(1)
        ;
}
__generic<T : __BuiltinLogicalType, let N : int>
bool testVShuffleX() {
    typealias gvec = vector<T, N>;

    return true
        && subgroupShuffle(gvec(T(1)), 1) == gvec(T(1))
        && subgroupShuffleXor(gvec(T(1)), 1) == gvec(T(1))
        ;
}

__generic<T : __BuiltinFloatingPointType>
bool test1ShuffleX() {
    return true
        && subgroupShuffle(T(1), 1) == T(1)
        && subgroupShuffleXor(T(1), 1) == T(1)
        ;
}
__generic<T : __BuiltinFloatingPointType, let N : int>
bool testVShuffleX() {
    typealias gvec = vector<T, N>;

    return true
        && subgroupShuffle(gvec(T(1)), 1) == gvec(T(1))
        && subgroupShuffleXor(gvec(T(1)), 1) == gvec(T(1))
        ;
}
bool testShuffleX() {
    return true
        && test1ShuffleX<float>()
        && testVShuffleX<float, 2>()
        && testVShuffleX<float, 3>()
        && testVShuffleX<float, 4>()
        //&& test1ShuffleX<double>() // hang
        //&& testVShuffleX<double, 2>()
        //&& testVShuffleX<double, 3>()
        //&& testVShuffleX<double, 4>()
        //&& test1ShuffleX<half>() // shader_subgroup_preamble --> __requireGLSLExtension fails
        //&& testVShuffleX<half, 2>()
        //&& testVShuffleX<half, 3>()
        //&& testVShuffleX<half, 4>()
        && test1ShuffleX<int>()
        && testVShuffleX<int, 2>()
        && testVShuffleX<int, 3>()
        && testVShuffleX<int, 4>()
        //&& test1ShuffleX<int8_t>() // shader_subgroup_preamble --> __requireGLSLExtension fails
        //&& testVShuffleX<int8_t, 2>()
        //&& testVShuffleX<int8_t, 3>()
        //&& testVShuffleX<int8_t, 4>()
        //&& test1ShuffleX<int16_t>() // shader_subgroup_preamble --> __requireGLSLExtension fails
        //&& testVShuffleX<int16_t, 2>()
        //&& testVShuffleX<int16_t, 3>()
        //&& testVShuffleX<int16_t, 4>()
        //&& test1ShuffleX<int64_t>() // shader_subgroup_preamble --> __requireGLSLExtension fails
        //&& testVShuffleX<int64_t, 2>()
        //&& testVShuffleX<int64_t, 3>()
        //&& testVShuffleX<int64_t, 4>()
        && test1ShuffleX<uint>()
        && testVShuffleX<uint, 2>()
        && testVShuffleX<uint, 3>()
        && testVShuffleX<uint, 4>()
        && test1ShuffleX<bool>()
        && testVShuffleX<bool, 2>()
        && testVShuffleX<bool, 3>()
        && testVShuffleX<bool, 4>()
        ;
}


void computeMain()
{

    outputBuffer.data[0] = true
        && testShuffleX()
        ;

    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1
}

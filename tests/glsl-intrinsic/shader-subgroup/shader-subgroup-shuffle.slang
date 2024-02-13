//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv

// not testing hlsl due to missing impl
//T-EST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl -DTARGET_HLSL
// not testing cuda due to missing impl
//T-EST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda -DTARGET_CUDA 
// not testing cpp due to missing impl
//T-EST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

// __requireGLSLExtension does not add an extension specified with GLSL, as a result half/i8/i16/i64 all don't run and complain about missing extension
//#define TEST_when_requireGLSLExtension_works

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
#if !defined(TARGET_CUDA) && !defined(TARGET_HLSL)
        && subgroupShuffleXor(T(1), 1) == T(1)
#endif // #if !defined(TARGET_CUDA) && !defined(TARGET_HLSL)
        ;
}
__generic<T : __BuiltinLogicalType, let N : int>
bool testVShuffleX() {
    typealias gvec = vector<T, N>;

    return true
        && subgroupShuffle(gvec(T(1)), 1) == gvec(T(1))
#if !defined(TARGET_CUDA) && !defined(TARGET_HLSL)
        && subgroupShuffleXor(gvec(T(1)), 1) == gvec(T(1))
#endif // #if !defined(TARGET_CUDA) && !defined(TARGET_HLSL)
        ;
}

__generic<T : __BuiltinFloatingPointType>
bool test1ShuffleX() {
    return true
        && subgroupShuffle(T(1), 1) == T(1)
#if !defined(TARGET_CUDA) && !defined(TARGET_HLSL)
        && subgroupShuffleXor(T(1), 1) == T(1)
#endif // #if !defined(TARGET_CUDA) && !defined(TARGET_HLSL)
        ;
}
__generic<T : __BuiltinFloatingPointType, let N : int>
bool testVShuffleX() {
    typealias gvec = vector<T, N>;

    return true
        && subgroupShuffle(gvec(T(1)), 1) == gvec(T(1))
#if !defined(TARGET_CUDA) && !defined(TARGET_HLSL)
        && subgroupShuffleXor(gvec(T(1)), 1) == gvec(T(1))
#endif // #if !defined(TARGET_CUDA) && !defined(TARGET_HLSL)
        ;
}
bool testShuffleX() {
    return true
        && test1ShuffleX<float>()
        && testVShuffleX<float, 2>()
        && testVShuffleX<float, 3>()
        && testVShuffleX<float, 4>()
        && test1ShuffleX<double>() // WARNING: intel GPU's lack FP64 support
        && testVShuffleX<double, 2>()
        && testVShuffleX<double, 3>()
        && testVShuffleX<double, 4>()
#ifdef TEST_when_requireGLSLExtension_works
        && test1ShuffleX<half>() 
        && testVShuffleX<half, 2>()
        && testVShuffleX<half, 3>()
        && testVShuffleX<half, 4>()
#endif // #ifdef TEST_when_requireGLSLExtension_works
        && test1ShuffleX<int>()
        && testVShuffleX<int, 2>()
        && testVShuffleX<int, 3>()
        && testVShuffleX<int, 4>()
#ifdef TEST_when_requireGLSLExtension_works
        && test1ShuffleX<int8_t>() 
        && testVShuffleX<int8_t, 2>()
        && testVShuffleX<int8_t, 3>()
        && testVShuffleX<int8_t, 4>()
        && test1ShuffleX<int16_t>() 
        && testVShuffleX<int16_t, 2>()
        && testVShuffleX<int16_t, 3>()
        && testVShuffleX<int16_t, 4>()
        && test1ShuffleX<int64_t>() 
        && testVShuffleX<int64_t, 2>()
        && testVShuffleX<int64_t, 3>()
        && testVShuffleX<int64_t, 4>()
#endif // #ifdef TEST_when_requireGLSLExtension_works
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

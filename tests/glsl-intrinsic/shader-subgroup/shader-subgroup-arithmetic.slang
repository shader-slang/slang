//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv
//TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl -DTARGET_HLSL
//TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda -DTARGET_CUDA 

// not testing cpp due to missing impl
//T-EST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

#version 430

// breaks on all gpu's tested (Nvidia & Intel) -- GLSL does not work, SPIR-V does work
//#define TEST_when_glsl_subgroupInclusiveXor_is_not_bugged
//#define TEST_when_glsl_subgroupInclusiveAdd_is_not_bugged

// __requireGLSLExtension does not work with GLSL, as a result half/i8/i16/i64 all don't run
//#define TEST_when_requireGLSLExtension_works 

precision highp float;
precision highp int;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

#define local_size_x_v 32
layout(local_size_x = local_size_x_v) in;

__generic<T : __BuiltinLogicalType>
bool test1Logical() {
    return true
#if !defined(TARGET_HLSL) && !defined(TARGET_CUDA)
        && subgroupAnd(T(1)) == T(1)
        && subgroupOr(T(1)) == T(1)
        && subgroupXor(T(1)) == T(0) 
        && subgroupInclusiveAnd(T(1)) == T(1)
        && subgroupInclusiveOr(T(1)) == T(1)
#ifdef TEST_when_glsl_subgroupInclusiveXor_is_not_bugged
        && subgroupInclusiveXor(T(1)) == T(0)
#endif // #ifdef TEST_when_glsl_subgroupInclusiveXor_is_not_bugged
        && subgroupExclusiveAnd(T(1)) == T(1)
        && subgroupExclusiveOr(T(1)) == T(1)
        && subgroupExclusiveXor(T(1)) == T(1)
#endif // #if !defined(TARGET_HLSL) && !defined(TARGET_CUDA)
        ;
}

__generic<T : __BuiltinLogicalType, let N : int>
bool testVLogical() {
    typealias gvec = vector<T, N>;
    
    return true
#if !defined(TARGET_HLSL) && !defined(TARGET_CUDA)
        && subgroupAnd(gvec(T(1))) == gvec(T(1))
        && subgroupOr(gvec(T(1))) == gvec(T(1))
        && subgroupXor(gvec(T(1))) == gvec(T(0))
        && subgroupInclusiveAnd(gvec(T(1))) == gvec(T(1))
        && subgroupInclusiveOr(gvec(T(1))) == gvec(T(1))
#ifdef TEST_when_glsl_subgroupInclusiveXor_is_not_bugged
        && subgroupInclusiveXor(gvec(T(1))) == gvec(T(0))
#endif // #ifdef TEST_when_glsl_subgroupInclusiveXor_is_not_bugged
        && subgroupExclusiveAnd(gvec(T(1))) == gvec(T(1))
        && subgroupExclusiveOr(gvec(T(1))) == gvec(T(1))
        && subgroupExclusiveXor(gvec(T(1))) == gvec(T(1))
#endif // #if !defined(TARGET_HLSL) && !defined(TARGET_CUDA)
        ;
}

bool testLogical() {
    return true
        && test1Logical<int>()
        && testVLogical<int, 2>()
        && testVLogical<int, 3>()
        && testVLogical<int, 4>()
#ifdef TEST_when_requireGLSLExtension_works
        && test1Logical<int8_t>()
        && testVLogical<int8_t, 2>()
        && testVLogical<int8_t, 3>()
        && testVLogical<int8_t, 4>()
        && test1Logical<int16_t>()
        && testVLogical<int16_t, 2>()
        && testVLogical<int16_t, 3>()
        && testVLogical<int16_t, 4>()
        && test1Logical<int64_t>()
        && testVLogical<int64_t, 2>()
        && testVLogical<int64_t, 3>()
        && testVLogical<int64_t, 4>()
#endif //#ifdef TEST_when_requireGLSLExtension_works
        && test1Logical<uint>()
        && testVLogical<uint, 2>()
        && testVLogical<uint, 3>()
        && testVLogical<uint, 4>()
        && test1Logical<bool>()
        && testVLogical<bool, 2>()
        && testVLogical<bool, 3>()
        && testVLogical<bool, 4>()
        ;
}

__generic<T : __BuiltinArithmeticType>
bool test1Arithmetic() {
    return true
        && subgroupAdd(T(1)) == T(local_size_x_v) // 32
        && subgroupMul(T(1)) == T(1)
        && subgroupMin(T(1)) == T(1)
        && subgroupMin(T(1)) == T(1)
        && subgroupExclusiveAdd(T(1)) == T(3)
        && subgroupExclusiveMul(T(1)) == T(1)
        && subgroupExclusiveMin(T(1)) == T(1)
        && subgroupExclusiveMax(T(1)) == T(1)
#ifdef TEST_when_glsl_subgroupInclusiveAdd_is_not_bugged
           && subgroupInclusiveAdd(T(1)) == T(4)
#endif // #ifdef TEST_when_glsl_subgroupInclusiveAdd_is_not_bugged
        && subgroupInclusiveMul(T(1)) == T(1)
        && subgroupInclusiveMin(T(1)) == T(1)
        && subgroupInclusiveMax(T(1)) == T(1)
        ;
}
__generic<T : __BuiltinArithmeticType, let N : int>
bool testVArithmetic() {
    typealias gvec = vector<T, N>;

    return true
        && subgroupAdd(gvec(T(1))) == gvec(T(local_size_x_v)) // 32
        && subgroupMul(gvec(T(1))) == gvec(T(1))
        && subgroupMin(gvec(T(1))) == gvec(T(1))
        && subgroupMin(gvec(T(1))) == gvec(T(1))
        && subgroupExclusiveAdd(gvec(T(1))) == gvec(T(3))
        && subgroupExclusiveMul(gvec(T(1))) == gvec(T(1))
        && subgroupExclusiveMin(gvec(T(1))) == gvec(T(1))
        && subgroupExclusiveMax(gvec(T(1))) == gvec(T(1))
#ifdef TEST_when_glsl_subgroupInclusiveAdd_is_not_bugged
        && subgroupInclusiveAdd(gvec(T(1))) == gvec(T(4))
#endif // #ifdef TEST_when_glsl_subgroupInclusiveAdd_is_not_bugged 
        && subgroupInclusiveMul(gvec(T(1))) == gvec(T(1))
        && subgroupInclusiveMin(gvec(T(1))) == gvec(T(1))
        && subgroupInclusiveMax(gvec(T(1))) == gvec(T(1))
        ;
}

bool testArithmetic() {
    return true
        && test1Arithmetic<float>()
        && testVArithmetic<float, 2>()
        && testVArithmetic<float, 3>()
        && testVArithmetic<float, 4>()
        && test1Arithmetic<double>() // WARNING: intel GPU's lack FP64 support
        && testVArithmetic<double, 2>()
        && testVArithmetic<double, 3>()
        && testVArithmetic<double, 4>()
#ifdef TEST_when_requireGLSLExtension_works
        && test1Arithmetic<half>()
        && testVArithmetic<half, 2>()
        && testVArithmetic<half, 3>()
        && testVArithmetic<half, 4>()
#endif // #ifdef TEST_when_requireGLSLExtension_works
        && test1Arithmetic<int>()
        && testVArithmetic<int, 2>()
        && testVArithmetic<int, 3>()
        && testVArithmetic<int, 4>()
#ifdef TEST_when_requireGLSLExtension_works
        && test1Arithmetic<int8_t>() 
        && testVArithmetic<int8_t, 2>()
        && testVArithmetic<int8_t, 3>()
        && testVArithmetic<int8_t, 4>()
        && test1Arithmetic<int16_t>() 
        && testVArithmetic<int16_t, 2>()
        && testVArithmetic<int16_t, 3>()
        && testVArithmetic<int16_t, 4>()
        && test1Arithmetic<int64_t>() 
        && testVArithmetic<int64_t, 2>()
        && testVArithmetic<int64_t, 3>()
        && testVArithmetic<int64_t, 4>()
#endif // TEST_when_requireGLSLExtension_works
        && test1Arithmetic<uint>()
        && testVArithmetic<uint, 2>()
        && testVArithmetic<uint, 3>()
        && testVArithmetic<uint, 4>()
        ;
}

void computeMain()
{

    bool res0 = true
            && testLogical()
            ;
    bool res1 = true
            && testArithmetic()
            ;
    if (gl_LocalInvocationID.x == 3) {
        // seperate so if there is an erroneous error the "major"
        // tests are issolated into 2 branches without polluting the
        // file with a bunch of individual test values
        outputBuffer.data[0] = res0;
        outputBuffer.data[1] = res1;
    }

    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1
    // BUF-NEXT: 1
}

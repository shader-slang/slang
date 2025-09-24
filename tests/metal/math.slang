//DISABLE_TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal
//DISABLE_TEST:SIMPLE(filecheck=GLSL): -stage compute -entry computeMain -target glsl
//DISABLE_TEST:SIMPLE(filecheck=GLSL_SPIRV): -stage compute -entry computeMain -target spirv -emit-spirv-via-glsl
//DISABLE_TEST:SIMPLE(filecheck=SPIR): -stage compute -entry computeMain -target spirv -emit-spirv-directly
//DISABLE_TEST:SIMPLE(filecheck=HLSL): -stage compute -entry computeMain -target hlsl
//DISABLE_TEST:SIMPLE(filecheck=CUDA): -stage compute -entry computeMain -target cuda
//DISABLE_TEST:SIMPLE(filecheck=CPP):  -stage compute -entry computeMain -target cpp

//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-via-glsl
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-directly
//DISABLE_TEST:SIMPLE(filecheck=METALLIB): -target metallib

//TEST(compute, metal):COMPARE_COMPUTE:-metal -compute -output-using-type -xslang -DMETAL_COMPUTE

//TEST_INPUT:ubuffer(data=[0 1 -1], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT: ubuffer(data=[0], stride=4, count=449):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// METALLIB: define void @computeMain

// It is unclear why "nextafter" is not working for Metal.
#define TEST_WHEN_nextafter_WORKS 0

__generic<T:__BuiltinFloatingPointType>
void Test_Scalar(inout int i)
{
    // METAL-LABEL: Test_Scalar
    const T zero = T(inputBuffer[0]);
    const T one = T(inputBuffer[1]);

    const int zeroInt = int(inputBuffer[0]);

    T outFloat1, outFloat2;
    int outInt;

    bool voidResult = true;

    // METAL: sincos(
    // METAL-NOT: sincos(
    sincos<T>(zero, outFloat1, outFloat2);
    //voidResult = voidResult && zero == outFloat1 && one == outFloat2;
    outputBuffer[i++] = true
        && zero == outFloat1

    ;outputBuffer[i++] = true
        && one == outFloat2

    //return voidResult
        // METAL: acos(
        // METALLIB: acos.f32
    ;outputBuffer[i++] = true
        && zero == acos<T>(one)

        // METAL: acosh(
        // METALLIB: acosh.f32
    ;outputBuffer[i++] = true
        && zero == acosh<T>(one)

        // METAL: asin(
        // METALLIB: asin.f32
    ;outputBuffer[i++] = true
        && zero == asin<T>(zero)

        // METAL: asinh(
        // METALLIB: asinh.f32
    ;outputBuffer[i++] = true
        && zero == asinh<T>(zero)

        // METAL: atan(
        // METALLIB: atan.f32
    ;outputBuffer[i++] = true
        && zero == atan<T>(zero)

        // METAL: atan2(
        // METALLIB: atan2.f32
    ;outputBuffer[i++] = true
        && zero == atan2<T>(zero, one)

        // METAL: atanh(
        // METALLIB: atanh.f32
    ;outputBuffer[i++] = true
        && zero == atanh<T>(zero)

        // METAL: ceil(
        // METALLIB: ceil.f32
    ;outputBuffer[i++] = true
        && zero == ceil<T>(zero)

        // METAL: copysign(
        // METALLIB: bitcast float
    ;outputBuffer[i++] = true
        && zero == copysign<T>(zero, zero)

        // METAL: cos(
        // METALLIB: cos.f32
    ;outputBuffer[i++] = true
        && one == cos<T>(zero)

        // METAL: cosh(
        // METALLIB: cosh.f32
    ;outputBuffer[i++] = true
        && one == cosh<T>(zero)

        // METAL: cospi(
        // METALLIB: cospi.f32
    ;outputBuffer[i++] = true
        && one == cospi<T>(zero)

    ;outputBuffer[i++] = reinterpret<uint>(cospi<T>(zero))

        // METAL: divide(
        // METALLIB: fdiv
    ;outputBuffer[i++] = true
        && zero == divide<T>(zero, one)

        // METAL: exp(
        // METALLIB: exp.f32
    ;outputBuffer[i++] = true
        && one == exp<T>(zero)

        // METAL: exp2(
        // METALLIB: exp2.f32
    ;outputBuffer[i++] = true
        && one == exp2<T>(zero)

        // METAL: exp10(
        // METALLIB: exp10.f32
    ;outputBuffer[i++] = true
        && one == exp10<T>(zero)

        // METAL: fabs(
        // METALLIB: fabs.f32
    ;outputBuffer[i++] = true
        && zero == fabs<T>(zero)

        // METAL: abs(
    ;outputBuffer[i++] = true
        && zero == abs<T>(zero)

        // METAL: fdim(
    ;outputBuffer[i++] = true
        && zero == fdim<T>(zero, zero)

        // METAL: floor(
        // METALLIB: floor.f32
    ;outputBuffer[i++] = true
        && zero == floor<T>(zero)

        // METAL: fma(
        // METALLIB: fma.f32
    ;outputBuffer[i++] = true
        && zero == fma(zero, zero, zero)

        // METAL: fmax(
        // METALLIB: fmax.f32
    ;outputBuffer[i++] = true
        && zero == fmax<T>(zero, zero)

        // METAL: max(
    ;outputBuffer[i++] = true
        && zero == max<T>(zero, zero)

        // METAL: fmax3(
        // METALLIB: fmax3.f32
    ;outputBuffer[i++] = true
        && zero == fmax3<T>(zero, zero, zero)

        // METAL: max3(
    ;outputBuffer[i++] = true
        && zero == max3<T>(zero, zero, zero)

        // METAL: fmedian3(
        // METALLIB: fmedian3.f32
    ;outputBuffer[i++] = true
        && zero == fmedian3<T>(zero, zero, zero)

        // METAL: median3(
    ;outputBuffer[i++] = true
        && zero == median3<T>(zero, zero, zero)

        // METAL: fmin(
        // METALLIB: fmin.f32
    ;outputBuffer[i++] = true
        && zero == fmin<T>(zero, zero)

        // METAL: min(
    ;outputBuffer[i++] = true
        && zero == min<T>(zero, zero)

        // METAL: fmin3(
        // METALLIB: fmin3.f32
    ;outputBuffer[i++] = true
        && zero == fmin3<T>(zero, zero, zero)

        // METAL: min3(
    ;outputBuffer[i++] = true
        && zero == min3<T>(zero, zero, zero)

        // METAL-COUNT-2: fmod(
        // METALLIB-COUNT-2: fmod.f32
    ;outputBuffer[i++] = true
        && zero == fmod<T>(zero, one)

        // METAL: fract(
        // METALLIB: fract.f32
    ;outputBuffer[i++] = true
        && zero == fract<T>(zero)

        // METAL: frexp(
        // METALLIB: frexp_float
    ;outputBuffer[i++] = true
        && zero == frexp<T>(zero, outInt) && zeroInt == outInt

        // METAL: ldexp(
        // METALLIB: ldexp.f32
    ;outputBuffer[i++] = true
        && zero == ldexp<T>(zero, zeroInt)

        // METAL: log(
        // METALLIB: log.f32
    ;outputBuffer[i++] = true
        && zero == log<T>(one)

        // METAL: log2(
        // METALLIB: log2.f32
    ;outputBuffer[i++] = true
        && zero == log2<T>(one)

        // METAL: log10(
        // METALLIB: log10.f32
    ;outputBuffer[i++] = true
        && zero == log10<T>(one)

        // METAL: modf(
    ;outputBuffer[i++] = true
        && zero == modf<T>(zero, outFloat1)

#if TEST_WHEN_nextafter_WORKS
        // M-ETAL: nextafter(
    ;outputBuffer[i++] = true
        && zero == nextafter<T>(zero, zero)
#endif

        // METAL: pow(
        // METALLIB: pow.f32
    ;outputBuffer[i++] = true
        && zero == pow<T>(zero, one)

        // METAL: powr(
        // METALLIB: powr.f32
    ;outputBuffer[i++] = true
        && zero == powr<T>(zero, one)

        // METAL: rint(
        // METALLIB: rint.f32
    ;outputBuffer[i++] = true
        && zero == rint<T>(zero)

        // METAL: round(
        // METALLIB: round.f32
    ;outputBuffer[i++] = true
        && zero == round<T>(zero)

        // METAL: rsqrt(
        // METALLIB: rsqrt.f32
    ;outputBuffer[i++] = true
        && one == rsqrt<T>(one)

        // METAL: sin(
        // METALLIB: sin.f32
    ;outputBuffer[i++] = true
        && zero == sin<T>(zero)

        // METAL: sinh(
        // METALLIB: sinh.f32
    ;outputBuffer[i++] = true
        && zero == sinh<T>(zero)

        // METAL: sinpi(
        // METALLIB: sinpi.f32
    ;outputBuffer[i++] = true
        && zero == sinpi<T>(zero)

        // METAL: sqrt(
        // METALLIB: sqrt.f32
    ;outputBuffer[i++] = true
        && zero == sqrt<T>(zero)

        // METAL: tan(
        // METALLIB: tan.f32
    ;outputBuffer[i++] = true
        && zero == tan<T>(zero)

        // METAL: tanh(
        // METALLIB: tanh.f32
    ;outputBuffer[i++] = true
        && zero == tanh<T>(zero)

        // METAL: tanpi(
        // METALLIB: tanpi.f32
    ;outputBuffer[i++] = true
        && zero == tanpi<T>(zero)

        // METAL: trunc(
    ;outputBuffer[i++] = true
        && zero == trunc<T>(zero)
        ;

    // METALLIB: ret
}

__generic<T:__BuiltinFloatingPointType, let N : int>
void Test_Vector(inout int i)
{
    // METAL-LABEL: Test_Vector_0
    const vector<T,N> zero = T(inputBuffer[0]);
    const vector<T,N> one = T(inputBuffer[1]);

    const vector<int,N> zeroInt = int(inputBuffer[0]);

    vector<T,N> outFloat1, outFloat2;
    vector<int,N> outInt;

    bool voidResult = true;

    // METAL: sincos(
    // METAL-NOT: sincos(
    sincos<T>(zero, outFloat1, outFloat2);
    //voidResult = voidResult && zero == outFloat1 && one == outFloat2;
    outputBuffer[i++] = true
        && zero == outFloat1

    ;outputBuffer[i++] = true
        && one == outFloat2

    //return voidResult
        // METAL: acos(
        // METAL-NOT: acos(
    ;outputBuffer[i++] = true
        && zero == acos<T>(one)

        // METAL: acosh(
        // METAL-NOT: acosh(
    ;outputBuffer[i++] = true
        && zero == acosh<T>(one)

        // METAL: asin(
        // METAL-NOT: asin(
    ;outputBuffer[i++] = true
        && zero == asin<T>(zero)

        // METAL: asinh(
        // METAL-NOT: asinh(
    ;outputBuffer[i++] = true
        && zero == asinh<T>(zero)

        // METAL: atan(
        // METAL-NOT: atan(
    ;outputBuffer[i++] = true
        && zero == atan<T>(zero)

        // METAL: atan2(
        // METAL-NOT: atan2(
    ;outputBuffer[i++] = true
        && zero == atan2<T>(zero, one)

        // METAL: atanh(
        // METAL-NOT: atanh(
    ;outputBuffer[i++] = true
        && zero == atanh<T>(zero)

        // METAL: ceil(
        // METAL-NOT: ceil(
    ;outputBuffer[i++] = true
        && zero == ceil<T>(zero)

        // METAL: copysign(
        // METAL-NOT: copysign(
    ;outputBuffer[i++] = true
        && zero == copysign<T>(zero, zero)

        // METAL: cos(
        // METAL-NOT: cos(
    ;outputBuffer[i++] = true
        && one == cos<T>(zero)

        // METAL: cosh(
        // METAL-NOT: cosh(
    ;outputBuffer[i++] = true
        && one == cosh<T>(zero)

        // METAL: cospi(
        // METAL-NOT: cospi(
    ;outputBuffer[i++] = true
        && one == cospi<T>(zero)

    ;outputBuffer[i++] = reinterpret<uint>(cospi<T>(zero))

        // METAL: divide(
        // METAL-NOT: divide(
    ;outputBuffer[i++] = true
        && zero == divide<T>(zero, one)

        // METAL: exp(
        // METAL-NOT: exp(
    ;outputBuffer[i++] = true
        && one == exp<T>(zero)

        // METAL: exp2(
        // METAL-NOT: exp2(
    ;outputBuffer[i++] = true
        && one == exp2<T>(zero)

        // METAL: exp10(
        // METAL-NOT: exp10(
    ;outputBuffer[i++] = true
        && one == exp10<T>(zero)

        // METAL: fabs(
        // METAL-NOT: fabs(
    ;outputBuffer[i++] = true
        && zero == fabs<T>(zero)

        // METAL: abs(
        // METAL-NOT: abs(
    ;outputBuffer[i++] = true
        && zero == abs<T>(zero)

        // METAL: fdim(
        // METAL-NOT: fdim(
    ;outputBuffer[i++] = true
        && zero == fdim<T>(zero, zero)

        // METAL: floor(
        // METAL-NOT: floor(
    ;outputBuffer[i++] = true
        && zero == floor<T>(zero)

        // METAL: fma(
        // METAL-NOT: fma(
    ;outputBuffer[i++] = true
        && zero == fma(zero, zero, zero)

        // METAL: fmax(
        // METAL-NOT: fmax(
    ;outputBuffer[i++] = true
        && zero == fmax<T>(zero, zero)

        // METAL: max(
        // METAL-NOT: max(
    ;outputBuffer[i++] = true
        && zero == max<T>(zero, zero)

        // METAL: fmax3(
        // METAL-NOT: fmax3(
    ;outputBuffer[i++] = true
        && zero == fmax3<T>(zero, zero, zero)

        // METAL: max3(
        // METAL-NOT: max3(
    ;outputBuffer[i++] = true
        && zero == max3<T>(zero, zero, zero)

        // METAL: fmedian3(
        // METAL-NOT: fmedian3(
    ;outputBuffer[i++] = true
        && zero == fmedian3<T>(zero, zero, zero)

        // METAL: median3(
        // METAL-NOT: median3(
    ;outputBuffer[i++] = true
        && zero == median3<T>(zero, zero, zero)

        // METAL: fmin(
        // METAL-NOT: fmin(
    ;outputBuffer[i++] = true
        && zero == fmin<T>(zero, zero)

        // METAL: min(
        // METAL-NOT: min(
    ;outputBuffer[i++] = true
        && zero == min<T>(zero, zero)

        // METAL: fmin3(
        // METAL-NOT: fmin3(
    ;outputBuffer[i++] = true
        && zero == fmin3<T>(zero, zero, zero)

        // METAL: min3(
        // METAL-NOT: min3(
    ;outputBuffer[i++] = true
        && zero == min3<T>(zero, zero, zero)

        // METAL-COUNT-2: fmod(
        // METAL-NOT: fmod(
    ;outputBuffer[i++] = true
        && zero == fmod<T>(zero, one)

        // METAL: fract(
        // METAL-NOT: fract(
    ;outputBuffer[i++] = true
        && zero == fract<T>(zero)

        // METAL: frexp(
        // METAL-NOT: frexp(
    ;outputBuffer[i++] = true
        && zero == frexp<T>(zero, outInt) && all(zeroInt == outInt)

        // METAL: ldexp(
        // METAL-NOT: ldexp(
    ;outputBuffer[i++] = true
        && zero == ldexp<T>(zero, zeroInt)

        // METAL: log(
        // METAL-NOT: log(
    ;outputBuffer[i++] = true
        && zero == log<T>(one)

        // METAL: log2(
        // METAL-NOT: log2(
    ;outputBuffer[i++] = true
        && zero == log2<T>(one)

        // METAL: log10(
        // METAL-NOT: log10(
    ;outputBuffer[i++] = true
        && zero == log10<T>(one)

        // METAL: modf(
        // METAL-NOT: modf(
    ;outputBuffer[i++] = true
        && zero == modf<T>(zero, outFloat1)

#if TEST_WHEN_nextafter_WORKS
        // M-ETAL: nextafter(
        // METAL-NOT: nextafter(
    ;outputBuffer[i++] = true
        && zero == nextafter<T>(zero, zero)
#endif

        // METAL: pow(
        // METAL-NOT: pow(
    ;outputBuffer[i++] = true
        && zero == pow<T>(zero, one)

        // METAL: powr(
        // METAL-NOT: powr(
    ;outputBuffer[i++] = true
        && zero == powr<T>(zero, one)

        // METAL: rint(
        // METAL-NOT: rint(
    ;outputBuffer[i++] = true
        && zero == rint<T>(zero)

        // METAL: round(
        // METAL-NOT: round(
    ;outputBuffer[i++] = true
        && zero == round<T>(zero)

        // METAL: rsqrt(
        // METAL-NOT: rsqrt(
    ;outputBuffer[i++] = true
        && one == rsqrt<T>(one)

        // METAL: sin(
        // METAL-NOT: sin(
    ;outputBuffer[i++] = true
        && zero == sin<T>(zero)

        // METAL: sinh(
        // METAL-NOT: sinh(
    ;outputBuffer[i++] = true
        && zero == sinh<T>(zero)

        // METAL: sinpi(
        // METAL-NOT: sinpi(
    ;outputBuffer[i++] = true
        && zero == sinpi<T>(zero)

        // METAL: sqrt(
        // METAL-NOT: sqrt(
    ;outputBuffer[i++] = true
        && zero == sqrt<T>(zero)

        // METAL: tan(
        // METAL-NOT: tan(
    ;outputBuffer[i++] = true
        && zero == tan<T>(zero)

        // METAL: tanh(
        // METAL-NOT: tanh(
    ;outputBuffer[i++] = true
        && zero == tanh<T>(zero)

        // METAL: tanpi(
        // METAL-NOT: tanpi(
    ;outputBuffer[i++] = true
        && zero == tanpi<T>(zero)

        // METAL: trunc(
        // METAL-NOT: trunc(
    ;outputBuffer[i++] = true
        && zero == trunc<T>(zero)
        ;

    // METAL-LABEL: Test_Vector_1
}

[numthreads(1,1,1)]
void computeMain()
{
    // GLSL: void main(
    // GLSL_SPIRV: OpEntryPoint
    // SPIR: OpEntryPoint
    // HLSL: void computeMain(
    // CUDA: void computeMain(
    // CPP: void _computeMain(

#if 0
    bool result = true
        && Test_Scalar<float>()
        && Test_Vector<float, 2>()
        && Test_Vector<float, 3>()
        && Test_Vector<float, 4>()
        && Test_Scalar<half>()
        && Test_Vector<half, 2>()
        && Test_Vector<half, 3>()
        && Test_Vector<half, 4>()
        ;

    // BUF: 1
    outputBuffer[0] = int(result);
#endif
    int j=0;
    int i=8;

    Test_Scalar<float>(i);
    outputBuffer[j++] = i;
    Test_Vector<float, 2>(i);
    outputBuffer[j++] = i;
    Test_Vector<float, 3>(i);
    outputBuffer[j++] = i;
    Test_Vector<float, 4>(i);
    outputBuffer[j++] = i;
    Test_Scalar<half>(i);
    outputBuffer[j++] = i;
    Test_Vector<half, 2>(i);
    outputBuffer[j++] = i;
    Test_Vector<half, 3>(i);
    outputBuffer[j++] = i;
    Test_Vector<half, 4>(i);
    outputBuffer[j++] = i;

}

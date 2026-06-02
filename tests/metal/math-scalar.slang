//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal
//TEST:SIMPLE(filecheck=GLSL): -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=GLSL_SPIRV): -stage compute -entry computeMain -target spirv -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=SPIR): -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSL): -stage compute -entry computeMain -target hlsl
//TEST:SIMPLE(filecheck=CUDA): -stage compute -entry computeMain -target cuda
//TEST:SIMPLE(filecheck=CPP):  -stage compute -entry computeMain -target cpp

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-directly
//TEST:SIMPLE(filecheck=METALLIB): -target metallib

//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -output-using-type -xslang -DMETAL_COMPUTE

//TEST_INPUT:ubuffer(data=[0 1 -1], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// METALLIB: define void @computeMain

// It is unclear why "nextafter" is not working for Metal.
#define TEST_WHEN_nextafter_WORKS 0

// NOTE: This test is mainly equality comparisons of math functions results
// against a precise expected value, but the Metal spec defines a minimum
// accuracy for many of these math functions such that a range of results may
// be allowed, presumably corresponding to different generations of hardware.
// Exact comparisons are preferred here for simplicity's sake, but in cases
// where, e.g., an M1 may yield a different result from an M4, this test checks
// that the value falls within the documented range instead.

__generic<T:__BuiltinFloatingPointType>
bool fuzzyCompare(const T value, const T expected, const T epsilon)
{
    return
        value <= (expected + epsilon) &&
        value >= (expected - epsilon);
}

__generic<T:__BuiltinFloatingPointType>
bool Test_Scalar()
{
    // METAL-LABEL: Test_Scalar
    const T zero = T(inputBuffer[0]);
    const T one = T(inputBuffer[1]);

    const int zeroInt = int(inputBuffer[0]);

    const T EPS_E2N13 = T(0.0001220703125); // 2^-13

    T outFloat1, outFloat2;
    int outInt;

    bool voidResult = true;

    // METAL: sincos(
    // METAL-NOT: sincos(
    sincos<T>(zero, outFloat1, outFloat2);
    voidResult = voidResult && zero == outFloat1 && one == outFloat2;

    return voidResult
        // METAL: acos(
        // METALLIB: acos.f32
        && zero == acos<T>(one)

        // METAL: acosh(
        // METALLIB: acosh.f32
        && zero == acosh<T>(one)

        // METAL: asin(
        // METALLIB: asin.f32
        && zero == asin<T>(zero)

        // METAL: asinh(
        // METALLIB: asinh.f32
        && zero == asinh<T>(zero)

        // METAL: atan(
        // METALLIB: atan.f32
        && zero == atan<T>(zero)

        // METAL: atan2(
        // METALLIB: atan2.f32
        && zero == atan2<T>(zero, one)

        // METAL: atanh(
        // METALLIB: atanh.f32
        && zero == atanh<T>(zero)

        // METAL: ceil(
        // METALLIB: ceil.f32
        && zero == ceil<T>(zero)

        // METAL: copysign(
        // METALLIB: bitcast float
        && zero == copysign<T>(zero, zero)

        // METAL: cos(
        // METALLIB: cos.f32
        && one == cos<T>(zero)

        // METAL: cosh(
        // METALLIB: cosh.f32
        && one == cosh<T>(zero)

        // METAL: cospi(
        // METALLIB: cospi.f32
        && fuzzyCompare<T>(cospi<T>(zero), one, EPS_E2N13)

        // METAL: divide(
        // METALLIB: fdiv
        && zero == divide<T>(zero, one)

        // METAL: exp(
        // METALLIB: exp.f32
        && one == exp<T>(zero)

        // METAL: exp2(
        // METALLIB: exp2.f32
        && one == exp2<T>(zero)

        // METAL: exp10(
        // METALLIB: exp10.f32
        && one == exp10<T>(zero)

        // METAL: fabs(
        // METALLIB: fabs.f32
        && zero == fabs<T>(zero)

        // METAL: abs(
        && zero == abs<T>(zero)

        // METAL: fdim(
        && zero == fdim<T>(zero, zero)

        // METAL: floor(
        // METALLIB: floor.f32
        && zero == floor<T>(zero)

        // METAL: fma(
        // METALLIB: fma.f32
        && zero == fma(zero, zero, zero)

        // METAL: fmax(
        // METALLIB: fmax.f32
        && zero == fmax<T>(zero, zero)

        // METAL: max(
        && zero == max<T>(zero, zero)

        // METAL: fmax3(
        // METALLIB: fmax3.f32
        && zero == fmax3<T>(zero, zero, zero)

        // METAL: max3(
        && zero == max3<T>(zero, zero, zero)

        // METAL: fmedian3(
        // METALLIB: fmedian3.f32
        && zero == fmedian3<T>(zero, zero, zero)

        // METAL: median3(
        && zero == median3<T>(zero, zero, zero)

        // METAL: fmin(
        // METALLIB: fmin.f32
        && zero == fmin<T>(zero, zero)

        // METAL: min(
        && zero == min<T>(zero, zero)

        // METAL: fmin3(
        // METALLIB: fmin3.f32
        && zero == fmin3<T>(zero, zero, zero)

        // METAL: min3(
        && zero == min3<T>(zero, zero, zero)

        // METAL-COUNT-2: fmod(
        // METALLIB-COUNT-2: fmod.f32
        && zero == fmod<T>(zero, one)

        // METAL: fract(
        // METALLIB: fract.f32
        && zero == fract<T>(zero)

        // METAL: frexp(
        // METALLIB: frexp_float
        && zero == frexp<T>(zero, outInt) && zeroInt == outInt

        // METAL: ldexp(
        // METALLIB: ldexp.f32
        && zero == ldexp<T>(zero, zeroInt)

        // METAL: log(
        // METALLIB: log.f32
        && zero == log<T>(one)

        // METAL: log2(
        // METALLIB: log2.f32
        && zero == log2<T>(one)

        // METAL: log10(
        // METALLIB: log10.f32
        && zero == log10<T>(one)

        // METAL: modf(
        && zero == modf<T>(zero, outFloat1)

#if TEST_WHEN_nextafter_WORKS
        // M-ETAL: nextafter(
        && zero == nextafter<T>(zero, zero)
#endif

        // METAL: pow(
        // METALLIB: pow.f32
        && zero == pow<T>(zero, one)

        // METAL: powr(
        // METALLIB: powr.f32
        && zero == powr<T>(zero, one)

        // METAL: rint(
        // METALLIB: rint.f32
        && zero == rint<T>(zero)

        // METAL: round(
        // METALLIB: round.f32
        && zero == round<T>(zero)

        // METAL: rsqrt(
        // METALLIB: rsqrt.f32
        && one == rsqrt<T>(one)

        // METAL: sin(
        // METALLIB: sin.f32
        && zero == sin<T>(zero)

        // METAL: sinh(
        // METALLIB: sinh.f32
        && zero == sinh<T>(zero)

        // METAL: sinpi(
        // METALLIB: sinpi.f32
        && zero == sinpi<T>(zero)

        // METAL: sqrt(
        // METALLIB: sqrt.f32
        && zero == sqrt<T>(zero)

        // METAL: tan(
        // METALLIB: tan.f32
        && zero == tan<T>(zero)

        // METAL: tanh(
        // METALLIB: tanh.f32
        && zero == tanh<T>(zero)

        // METAL: tanpi(
        // METALLIB: tanpi.f32
        && zero == tanpi<T>(zero)

        // METAL: trunc(
        && zero == trunc<T>(zero)
        ;

    // METALLIB: ret
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

    bool result = true
        && Test_Scalar<float>()
        && Test_Scalar<half>()
        ;

    // BUF: 1
    outputBuffer[0] = int(result);
}

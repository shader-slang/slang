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

__generic<T:__BuiltinFloatingPointType, let N : int>
bool fuzzyCompare(const vector<T,N> value, const vector<T,N> expected, const vector<T,N> epsilon)
{
    return all(
        value <= (expected + epsilon) &&
        value >= (expected - epsilon)
        );
}

__generic<T:__BuiltinFloatingPointType, let N : int>
bool Test_Vector()
{
    // METAL-LABEL: Test_Vector_0
    const vector<T,N> zero = T(inputBuffer[0]);
    const vector<T,N> one = T(inputBuffer[1]);

    const vector<int,N> zeroInt = int(inputBuffer[0]);

    const vector<T,N> EPS_E2N13 = T(0.0001220703125); // 2^-13

    vector<T,N> outFloat1, outFloat2;
    vector<int,N> outInt;

    bool voidResult = true;

    // METAL: sincos(
    // METAL-NOT: sincos(
    sincos<T>(zero, outFloat1, outFloat2);
    voidResult = voidResult && all(zero == outFloat1) && all(one == outFloat2);

    return voidResult
        // METAL: acos(
        // METAL-NOT: acos(
        && all(zero == acos<T>(one))

        // METAL: acosh(
        // METAL-NOT: acosh(
        && all(zero == acosh<T>(one))

        // METAL: asin(
        // METAL-NOT: asin(
        && all(zero == asin<T>(zero))

        // METAL: asinh(
        // METAL-NOT: asinh(
        && all(zero == asinh<T>(zero))

        // METAL: atan(
        // METAL-NOT: atan(
        && all(zero == atan<T>(zero))

        // METAL: atan2(
        // METAL-NOT: atan2(
        && all(zero == atan2<T>(zero, one))

        // METAL: atanh(
        // METAL-NOT: atanh(
        && all(zero == atanh<T>(zero))

        // METAL: ceil(
        // METAL-NOT: ceil(
        && all(zero == ceil<T>(zero))

        // METAL: copysign(
        // METAL-NOT: copysign(
        && all(zero == copysign<T>(zero, zero))

        // METAL: cos(
        // METAL-NOT: cos(
        && all(one == cos<T>(zero))

        // METAL: cosh(
        // METAL-NOT: cosh(
        && all(one == cosh<T>(zero))

        // METAL: cospi(
        // METAL-NOT: cospi(
        && fuzzyCompare<T,N>(cospi<T>(zero), one, EPS_E2N13)

        // METAL: divide(
        // METAL-NOT: divide(
        && all(zero == divide<T>(zero, one))

        // METAL: exp(
        // METAL-NOT: exp(
        && all(one == exp<T>(zero))

        // METAL: exp2(
        // METAL-NOT: exp2(
        && all(one == exp2<T>(zero))

        // METAL: exp10(
        // METAL-NOT: exp10(
        && all(one == exp10<T>(zero))

        // METAL: fabs(
        // METAL-NOT: fabs(
        && all(zero == fabs<T>(zero))

        // METAL: abs(
        // METAL-NOT: abs(
        && all(zero == abs<T>(zero))

        // METAL: fdim(
        // METAL-NOT: fdim(
        && all(zero == fdim<T>(zero, zero))

        // METAL: floor(
        // METAL-NOT: floor(
        && all(zero == floor<T>(zero))

        // METAL: fma(
        // METAL-NOT: fma(
        && all(zero == fma(zero, zero, zero))

        // METAL: fmax(
        // METAL-NOT: fmax(
        && all(zero == fmax<T>(zero, zero))

        // METAL: max(
        // METAL-NOT: max(
        && all(zero == max<T>(zero, zero))

        // METAL: fmax3(
        // METAL-NOT: fmax3(
        && all(zero == fmax3<T>(zero, zero, zero))

        // METAL: max3(
        // METAL-NOT: max3(
        && all(zero == max3<T>(zero, zero, zero))

        // METAL: fmedian3(
        // METAL-NOT: fmedian3(
        && all(zero == fmedian3<T>(zero, zero, zero))

        // METAL: median3(
        // METAL-NOT: median3(
        && all(zero == median3<T>(zero, zero, zero))

        // METAL: fmin(
        // METAL-NOT: fmin(
        && all(zero == fmin<T>(zero, zero))

        // METAL: min(
        // METAL-NOT: min(
        && all(zero == min<T>(zero, zero))

        // METAL: fmin3(
        // METAL-NOT: fmin3(
        && all(zero == fmin3<T>(zero, zero, zero))

        // METAL: min3(
        // METAL-NOT: min3(
        && all(zero == min3<T>(zero, zero, zero))

        // METAL: fmod(
        // METAL-NOT: fmod(
        && all(zero == fmod<T>(zero, one))

        // METAL: fract(
        // METAL-NOT: fract(
        && all(zero == fract<T>(zero))

        // METAL: frexp(
        // METAL-NOT: frexp(
        && all(zero == frexp<T>(zero, outInt)) && all(zeroInt == outInt)

        // METAL: ldexp(
        // METAL-NOT: ldexp(
        && all(zero == ldexp<T>(zero, zeroInt))

        // METAL: log(
        // METAL-NOT: log(
        && all(zero == log<T>(one))

        // METAL: log2(
        // METAL-NOT: log2(
        && all(zero == log2<T>(one))

        // METAL: log10(
        // METAL-NOT: log10(
        && all(zero == log10<T>(one))

        // METAL: modf(
        // METAL-NOT: modf(
        && all(zero == modf<T>(zero, outFloat1))

#if TEST_WHEN_nextafter_WORKS
        // M-ETAL: nextafter(
        // METAL-NOT: nextafter(
        && all(zero == nextafter<T>(zero, zero))
#endif

        // METAL: pow(
        // METAL-NOT: pow(
        && all(zero == pow<T>(zero, one))

        // METAL: powr(
        // METAL-NOT: powr(
        && all(zero == powr<T>(zero, one))

        // METAL: rint(
        // METAL-NOT: rint(
        && all(zero == rint<T>(zero))

        // METAL: round(
        // METAL-NOT: round(
        && all(zero == round<T>(zero))

        // METAL: rsqrt(
        // METAL-NOT: rsqrt(
        && all(one == rsqrt<T>(one))

        // METAL: sin(
        // METAL-NOT: sin(
        && all(zero == sin<T>(zero))

        // METAL: sinh(
        // METAL-NOT: sinh(
        && all(zero == sinh<T>(zero))

        // METAL: sinpi(
        // METAL-NOT: sinpi(
        && all(zero == sinpi<T>(zero))

        // METAL: sqrt(
        // METAL-NOT: sqrt(
        && all(zero == sqrt<T>(zero))

        // METAL: tan(
        // METAL-NOT: tan(
        && all(zero == tan<T>(zero))

        // METAL: tanh(
        // METAL-NOT: tanh(
        && all(zero == tanh<T>(zero))

        // METAL: tanpi(
        // METAL-NOT: tanpi(
        && all(zero == tanpi<T>(zero))

        // METAL: trunc(
        // METAL-NOT: trunc(
        && all(zero == trunc<T>(zero))
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

    bool result = true
        && Test_Vector<float, 2>()
        && Test_Vector<float, 3>()
        && Test_Vector<float, 4>()
        && Test_Vector<half, 2>()
        && Test_Vector<half, 3>()
        && Test_Vector<half, 4>()
        ;

    // BUF: 1
    outputBuffer[0] = int(result);
}

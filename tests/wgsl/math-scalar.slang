//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

RWStructuredBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

__generic<T:__BuiltinFloatingPointType>
bool Test_Scalar()
{
    // WGSL-LABEL: Test_Scalar

    const T zero = T(inputBuffer[0]);
    const T one = T(inputBuffer[1]);
    const int zeroInt = int(inputBuffer[0]);

    T outFloat1, outFloat2;
    int outInt;

    return true

        // WGSL: acos(
        && zero == acos<T>(one)

        // WGSL: acosh(
        && zero == acosh<T>(one)

        // WGSL: asin(
        && zero == asin<T>(zero)

        // WGSL: asinh(
        && zero == asinh<T>(zero)

        // WGSL: atan(
        && zero == atan<T>(zero)

        // WGSL: atan2(
        && zero == atan2<T>(zero, zero)

        // WGSL: atanh(
        && zero == atanh<T>(zero)

        // WGSL: ceil(
        && zero == ceil<T>(zero)

        // WGSL: cos(
        && one == cos<T>(zero)

        // WGSL: cosh(
        && one == cosh<T>(zero)

        // WGSL: exp(
        && one == exp<T>(zero)

        // WGSL: exp2(
        && one == exp2<T>(zero)

        // WGSL: abs(
        && zero == abs<T>(zero)

        // WGSL: floor(
        && zero == floor<T>(zero)

        // WGSL: fma(
        && zero == fma(zero, zero, zero)

        // WGSL: max(
        && zero == max<T>(zero, zero)

        // WGSL: min(
        && zero == min<T>(zero, zero)

        // WGSL: fract(
        && zero == fract<T>(zero)

        // WGSL: frexp(
        && zero == frexp<T>(zero, outInt) && zeroInt == outInt

        // WGSL: ldexp(
        && zero == ldexp<T>(zero, zeroInt)

        // WGSL: log(
        && zero == log<T>(one)

        // WGSL: log2(
        && zero == log2<T>(one)

        // WGSL: modf(
        && zero == modf<T>(zero, outFloat1)

        // WGSL: pow(
        && zero == pow<T>(zero, one)

        // WGSL: round(
        && zero == round<T>(zero)

        // WGSL: sin(
        && zero == sin<T>(zero)

        // WGSL: sinh(
        && zero == sinh<T>(zero)

        // WGSL: sqrt(
        && zero == sqrt<T>(zero)

        // WGSL: tan(
        && zero == tan<T>(zero)

        // WGSL: tanh(
        && zero == tanh<T>(zero)

        // WGSL: trunc(
        && zero == trunc<T>(zero)
        ;
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

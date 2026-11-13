//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

RWStructuredBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

__generic<T:__BuiltinFloatingPointType, let N : int>
bool Test_Vector()
{
    // WGSL-LABEL: Test_Vector_0
    const vector<T,N> zero = T(inputBuffer[0]);
    const vector<T,N> one = T(inputBuffer[1]);

    const vector<int,N> zeroInt = int(inputBuffer[0]);

    vector<T,N> outFloat1, outFloat2;
    vector<int,N> outInt;

    return true
        // WGSL: acos(
        // WGSL-NOT: acos(
        && all(zero == acos<T>(one))

        // WGSL: acosh(
        // WGSL-NOT: acosh(
        && all(zero == acosh<T>(one))

        // WGSL: asin(
        // WGSL-NOT: asin(
        && all(zero == asin<T>(zero))

        // WGSL: asinh(
        // WGSL-NOT: asinh(
        && all(zero == asinh<T>(zero))

        // WGSL: atan(
        // WGSL-NOT: atan(
        && all(zero == atan<T>(zero))

        // WGSL: atan2(
        // WGSL-NOT: atan2(
        && all(zero == atan2<T>(zero, zero))

        // WGSL: atanh(
        // WGSL-NOT: atanh(
        && all(zero == atanh<T>(zero))

        // WGSL: ceil(
        // WGSL-NOT: ceil(
        && all(zero == ceil<T>(zero))

        // WGSL: cos(
        // WGSL-NOT: cos(
        && all(one == cos<T>(zero))

        // WGSL: cosh(
        // WGSL-NOT: cosh(
        && all(one == cosh<T>(zero))

        // WGSL: exp(
        // WGSL-NOT: exp(
        && all(one == exp<T>(zero))

        // WGSL: exp2(
        // WGSL-NOT: exp2(
        && all(one == exp2<T>(zero))

        // WGSL: abs(
        // WGSL-NOT: abs(
        && all(zero == abs<T>(zero))

        // WGSL: floor(
        // WGSL-NOT: floor(
        && all(zero == floor<T>(zero))

        // WGSL: fma(
        // WGSL-NOT: fma(
        && all(zero == fma(zero, zero, zero))

        // WGSL: max(
        // WGSL-NOT: max(
        && all(zero == max<T>(zero, zero))

        // WGSL: min(
        // WGSL-NOT: min(
        && all(zero == min<T>(zero, zero))

        // WGSL: fract(
        // WGSL-NOT: fract(
        && all(zero == fract<T>(zero))

        // WGSL: frexp(
        // WGSL-NOT: frexp(
        && all(zero == frexp<T>(zero, outInt)) && all(zeroInt == outInt)

        // WGSL: ldexp(
        // WGSL-NOT: ldexp(
        && all(zero == ldexp<T>(zero, zeroInt))

        // WGSL: log(
        // WGSL-NOT: log(
        && all(zero == log<T>(one))

        // WGSL: log2(
        // WGSL-NOT: log2(
        && all(zero == log2<T>(one))

        // WGSL: modf(
        // WGSL-NOT: modf(
        && all(zero == modf<T>(zero, outFloat1))

        // WGSL: pow(
        // WGSL-NOT: pow(
        && all(zero == pow<T>(zero, one))

        // WGSL: inverseSqrt(
        // WGSL-NOT: inverseSqrt(
        && all(one == rsqrt<T>(one))

        // WGSL: ) % (
        // WGSL-NOT: ) % (
        && all(zero == fmod<T>(zero, one))

        // WGSL: round(
        // WGSL-NOT: round(
        && all(zero == round<T>(zero))

        // WGSL: sin(
        // WGSL-NOT: sin(
        && all(zero == sin<T>(zero))

        // WGSL: sinh(
        // WGSL-NOT: sinh(
        && all(zero == sinh<T>(zero))

        // WGSL: sqrt(
        // WGSL-NOT: sqrt(
        && all(zero == sqrt<T>(zero))

        // WGSL: tan(
        // WGSL-NOT: tan(
        && all(zero == tan<T>(zero))

        // WGSL: tanh(
        // WGSL-NOT: tanh(
        && all(zero == tanh<T>(zero))

        // WGSL: trunc(
        // WGSL-NOT: trunc(
        && all(zero == trunc<T>(zero))
        ;

    // WGSL-LABEL: Test_Vector_1
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

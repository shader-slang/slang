// Under `-fp-mode precise`, the CUDA output must keep `a * b + c` from being contracted into an
// FMA when the user compiles it with nvcc, whose default is `--fmad=true` (#14036). Scalar
// floating-point add, sub and mul are emitted as `__fadd_rn` / `__dmul_rn` / ..., which nvcc never
// contracts, and SLANG_CUDA_PRECISE_FP switches the prelude's vector and matrix operators to the
// same functions. An explicit `fma` stays fused. The default mode is unchanged.
// The FileCheck lanes cover the emitter. The compute lane checks that the precise code builds and
// gives the unfused values; it would pass without the intrinsics too, because render-test's NVRTC
// compile already gets `--fmad=false` under `-fp-mode precise`.

//TEST:SIMPLE(filecheck=PRECISE): -target cuda -entry computeMain -stage compute -fp-mode precise -line-directive-mode none
//TEST:SIMPLE(filecheck=DEFAULT): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type -Xslang -fp-mode -Xslang precise

// PRECISE: #define SLANG_CUDA_PRECISE_FP 1
// PRECISE-DAG: = __fmul_rn(
// PRECISE-DAG: __fadd_rn(
// PRECISE-DAG: __fsub_rn(
// PRECISE-DAG: __dadd_rn(__dmul_rn(
// PRECISE-DAG: F32_fma(

// The prelude may be emitted inline, so match the define and the calls, not the names alone.
// DEFAULT-NOT: #define SLANG_CUDA_PRECISE_FP 1
// DEFAULT-NOT: = __fadd_rn(
// DEFAULT-NOT: = __fmul_rn(

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=8):name=doubleBuffer
RWStructuredBuffer<double> doubleBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // a * b = 1 + 2^-11 + 2^-24, which rounds to 1 + 2^-11. Unfused, a * b + c is 0; an FMA
    // gives 2^-24.
    float t = float(tid.x);
    float a = 1.0 + 1.0 / 4096.0 + t;
    float b = a;
    float c = -(1.0 + 1.0 / 2048.0) - t;

    outputBuffer[0] = a * b + c;

    float2 a2 = float2(a, a);
    float2 c2 = float2(c, c);
    float2 r2 = a2 * a2 + c2;
    outputBuffer[1] = r2.x;
    outputBuffer[2] = r2.y;

    float2x2 m = float2x2(a, 0, 0, a);
    float2x2 mc = float2x2(c, 0, 0, c);
    float2x2 rm = m * m + mc;
    outputBuffer[3] = rm[0][0];

    outputBuffer[4] = (a * b - (-c)) * 1.0;

    // An explicit fma is still fused: 2^-24.
    outputBuffer[5] = fma(a, b, c) * 16777216.0;

    double da = 1.0l + 1.0l / 134217728.0l + double(t); // 1 + 2^-27
    double dc = -(1.0l + 1.0l / 67108864.0l) - double(t);
    doubleBuffer[0] = da * da + dc;
}

// BUF: 0.0
// BUF-NEXT: 0.0
// BUF-NEXT: 0.0
// BUF-NEXT: 0.0
// BUF-NEXT: 0.0
// BUF-NEXT: 1.0

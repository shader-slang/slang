// At SM 5.1 the HLSL emitter writes the same typed literals as at SM 6.6, and FXC compiles them.
// FXC has no 16-bit or 64-bit integers, its `half` is a 32-bit `float`, and its
// `RWByteAddressBuffer.Store` takes a `uint`, so each result is stored as its `uint` bits. Each
// literal feeds an operation whose result depends on the literal's type: a shift by a run-time
// amount is arithmetic only for a signed literal, and a product with a run-time `double` keeps the
// bits of `0.1L` and of the highest finite `double` only when the literal is a `double`. The dxbc
// and dx12 lanes need FXC, so they run on Windows only. The cpu lane cross-checks the buffer
// words; a vk lane cannot, because Vulkan does not require a multiplication to preserve an
// infinity, a NaN or the sign of `-0.0` unless the shader asks for it.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -profile cs_5_1
//TEST:SIMPLE(filecheck=DXBC):-target dxbc-assembly -entry computeMain -stage compute -profile cs_5_1
//TEST:SIMPLE(filecheck=DXBCDIV):-target dxbc-assembly -entry computeMain -stage compute -profile cs_5_1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -use-dxbc -profile cs_5_1 -render-feature double -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA 0xAAAAAAAA], stride=4):out,name=output
RWByteAddressBuffer output;

void storeDouble(uint offset, double value)
{
    uint low;
    uint high;
    asuint(value, low, high);
    output.Store(offset, low);
    output.Store(offset + 4, high);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Every operand below is 1, but it depends on the thread ID, so Slang does not fold the
    // operations and each literal reaches the HLSL.
    uint s = tid.x + 1;
    int one = int(s);
    float oneF = float(s);
    double oneD = double(s);
    half oneH = half(s);

    // HLSL-DAG: (-2147483647L - 1L) >> s_0
    // HLSL-DAG: 2147483647L >> s_0
    // HLSL-DAG: -12345L >> s_0
    // HLSL-DAG: (int(0) - one_0) >> 1L
    // HLSL-DAG: 4294967295U >> s_0
    // HLSL-DAG: 12345U >> s_0
    // HLSL-DAG: 0.10000000149011612f * oneF_0
    // HLSL-DAG: (1.0f / 0.0f) * oneF_0
    // HLSL-DAG: 0.10000000000000001L * oneD_0
    // HLSL-DAG: 1.79769313486231571e+308L * oneD_0
    // HLSL-DAG: (0.0L / 0.0L) * oneD_0
    // HLSL-DAG: 65504.0h * oneH_0
    // HLSL-DAG: (-1.0h / 0.0h) * oneH_0

    // DXBC-DAG: ishr
    // DXBC-DAG: ushr
    // DXBC-DAG: dmul

    // FXC folds each division by zero, so no `double` division needs the extended `double`
    // instructions.
    // DXBCDIV: cs_5_1
    // DXBCDIV-NOT: ddiv

    output.Store(0, uint(int.minValue >> s));
    output.Store(4, uint(int.maxValue >> s));
    output.Store(8, uint(-12345 >> s));
    output.Store(12, uint((0 - one) >> 1));
    output.Store(16, uint.maxValue >> s);
    output.Store(20, 12345U >> s);
    output.Store(24, asuint(0.1f * oneF));
    output.Store(28, asuint(-0.0f * oneF));
    output.Store(32, asuint((1.0f / 0.0f) * oneF));
    output.Store(36, asuint((-1.0f / 0.0f) * oneF));
    output.Store(40, asuint((0.0f / 0.0f) * oneF));
    storeDouble(44, 0.1l * oneD);
    storeDouble(52, 1.7976931348623157e+308l * oneD);
    storeDouble(60, -0.0l * oneD);
    storeDouble(68, double(1.0 / 0.0) * oneD);
    storeDouble(76, double(-1.0 / 0.0) * oneD);
    storeDouble(84, double(0.0 / 0.0) * oneD);
    output.Store(92, asuint(float(half(65504.0) * oneH)));
    output.Store(96, asuint(float(half(-0.0) * oneH)));
    output.Store(100, asuint(float(half(1.0 / 0.0) * oneH)));
    output.Store(104, asuint(float(half(-1.0 / 0.0) * oneH)));
    output.Store(108, asuint(float(half(0.0 / 0.0) * oneH)));

    // The NaN words accept a quiet NaN of either sign, because the sign and payload of a NaN that
    // a multiplication returns differ between targets. The last word is never written.
    // BUF: C0000000
    // BUF-NEXT: 3FFFFFFF
    // BUF-NEXT: FFFFE7E3
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: 7FFFFFFF
    // BUF-NEXT: 181C
    // BUF-NEXT: 3DCCCCCD
    // BUF-NEXT: 80000000
    // BUF-NEXT: 7F800000
    // BUF-NEXT: FF800000
    // BUF-NEXT: {{[7F]F[C-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]}}
    // BUF-NEXT: 9999999A
    // BUF-NEXT: 3FB99999
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: 7FEFFFFF
    // BUF-NEXT: 0
    // BUF-NEXT: 80000000
    // BUF-NEXT: 0
    // BUF-NEXT: 7FF00000
    // BUF-NEXT: 0
    // BUF-NEXT: FFF00000
    // BUF-NEXT: {{[0-9A-F]+}}
    // BUF-NEXT: {{[7F]FF[89A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]}}
    // BUF-NEXT: 477FE000
    // BUF-NEXT: 80000000
    // BUF-NEXT: 7F800000
    // BUF-NEXT: FF800000
    // BUF-NEXT: {{[7F]F[C-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]}}
    // BUF-NEXT: AAAAAAAA
}

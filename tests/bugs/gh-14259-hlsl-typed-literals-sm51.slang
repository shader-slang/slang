// At SM 5.1 the HLSL emitter writes the same typed literals as at SM 6.6, and FXC compiles them.
// FXC has no 16-bit or 64-bit integers, its `half` is a 32-bit `float`, and its
// `RWByteAddressBuffer.Store` takes a `uint`, so each result is stored as its `uint` bits. Each
// literal feeds an operation with a run-time value. For the integer and `double` literals, the
// result depends on the literal's type: a shift by a run-time amount is arithmetic only for a
// signed literal, and a product with a run-time `double` keeps the bits of `0.1L` only when the
// literal is a `double`. The `float` and `half` operations
// check that FXC accepts the literals and gives their values; FXC cannot tell a `half` from a
// `float`. The dxbc
// and dx12 lanes need FXC, so they run on Windows only. The cpu lane cross-checks the buffer
// words; a vk lane cannot, because Vulkan does not require a multiplication to preserve an
// infinity or a NaN unless the shader asks for it.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -profile cs_5_1
//TEST:SIMPLE(filecheck=DXBC):-target dxbc-assembly -entry computeMain -stage compute -profile cs_5_1
//TEST:SIMPLE(filecheck=DXBCDIV):-target dxbc-assembly -entry computeMain -stage compute -profile cs_5_1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -use-dxbc -profile cs_5_1 -render-feature double -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=H23):-dx12 -use-dxbc -profile cs_5_1 -render-feature double -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=H25):-dx12 -use-dxbc -profile cs_5_1 -render-feature double -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=H26):-dx12 -use-dxbc -profile cs_5_1 -render-feature double -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=H27):-dx12 -use-dxbc -profile cs_5_1 -render-feature double -shaderobj
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
    // HLSL-DAG: (0.0L / 0.0L) * oneD_0
    // HLSL-DAG: 65504.0h * oneH_0
    // HLSL-DAG: (-1.0h / 0.0h) * oneH_0

    // The DXBC lines check that FXC accepts the typed literals and compiles signed and unsigned
    // shifts and `double` products; they cannot tie an instruction to one literal. The buffer words
    // of the dx12 lane check the signedness of the integer literals and the precision of the
    // `double` literals.
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
    output.Store(32, asuint((1.0f / 0.0f) * oneF));
    output.Store(36, asuint((-1.0f / 0.0f) * oneF));
    output.Store(40, asuint((0.0f / 0.0f) * oneF));
    storeDouble(44, 0.1l * oneD);
    storeDouble(68, double(1.0 / 0.0) * oneD);
    storeDouble(76, double(-1.0 / 0.0) * oneD);
    storeDouble(84, double(0.0 / 0.0) * oneD);
    output.Store(92, asuint(float(half(65504.0) * oneH)));
    output.Store(100, asuint(float(half(1.0 / 0.0) * oneH)));
    output.Store(104, asuint(float(half(-1.0 / 0.0) * oneH)));
    output.Store(108, asuint(float(half(0.0 / 0.0) * oneH)));

    // On D3D12 with FXC, a stored `-0.0` literal does not keep its sign bit, and a product with
    // the highest finite `double` does not store its expected bits, so this test stores neither;
    // the SM 6.6 tests check both with dxc. Words 7, 13-16 and 24 stay unwritten.
    // The NaN words accept a quiet NaN of either sign, because the sign and payload of a NaN that
    // a multiplication returns differ between targets. The last word is never written.
    // BUF: C0000000
    // BUF-NEXT: 3FFFFFFF
    // BUF-NEXT: FFFFE7E3
    // BUF-NEXT: FFFFFFFF
    // BUF-NEXT: 7FFFFFFF
    // BUF-NEXT: 181C
    // BUF-NEXT: 3DCCCCCD
    // BUF-NEXT: AAAAAAAA
    // BUF-NEXT: 7F800000
    // BUF-NEXT: FF800000
    // BUF-NEXT: {{[7F]F[C-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]}}
    // BUF-NEXT: 9999999A
    // BUF-NEXT: 3FB99999
    // BUF-NEXT: AAAAAAAA
    // BUF-NEXT: AAAAAAAA
    // BUF-NEXT: AAAAAAAA
    // BUF-NEXT: AAAAAAAA
    // BUF-NEXT: 0
    // BUF-NEXT: 7FF00000
    // BUF-NEXT: 0
    // BUF-NEXT: FFF00000
    // BUF-NEXT: {{[0-9A-F]+}}
    // BUF-NEXT: {{[7F]FF[89A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]}}
    // BUF-NEXT: 477FE000
    // BUF-NEXT: AAAAAAAA
    // BUF-NEXT: 7F800000
    // BUF-NEXT: FF800000
    // BUF-NEXT: {{[7F]F[C-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]}}
    // BUF-NEXT: AAAAAAAA
    // Each Hn lane checks `half` word n on its own, so one run reports every `half` mismatch even
    // when an earlier word fails. Words 24 and 28 are never written.
    // H23: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: {{[0-9A-F]+}}
    // H23-NEXT: 477FE000
    // H25: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: {{[0-9A-F]+}}
    // H25-NEXT: 7F800000
    // H26: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: {{[0-9A-F]+}}
    // H26-NEXT: FF800000
    // H27: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[0-9A-F]+}}
    // H27-NEXT: {{[7F]F[C-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]}}
}

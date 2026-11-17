// Regression test for https://github.com/shader-slang/slang/issues/13762
//
// A `[ForceUnroll]` loop that divides by `4u - i` for `i < 4u` crashed slangc. The
// unroller folds the instructions of the step one past the last iteration (`i == 4`),
// where the divisor is the constant 0. The fold reported "divide by zero" through a
// diagnostic sink that `tryConstantFoldInst` never initialized. That step is never
// executed, so the code must compile, and it must not report a divide-by-zero error.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// The unrolled loops divide by 4, 3, 2 and 1 (a division by 1 may be folded away), and nothing
// divides by 0.
// SPIRV-NOT: error[E
// SPIRV: OpEntryPoint
// SPIRV: %[[D4:[A-Za-z0-9_]+]] = OpUDiv %uint %{{[A-Za-z0-9_]+}} %uint_4
// SPIRV-NEXT: %[[D3:[A-Za-z0-9_]+]] = OpUDiv %uint %[[D4]] %uint_3
// SPIRV-NEXT: %{{[A-Za-z0-9_]+}} = OpUDiv %uint %[[D3]] %uint_2
// SPIRV: %[[M4:[A-Za-z0-9_]+]] = OpUMod %uint %{{[A-Za-z0-9_]+}} %uint_4
// SPIRV-NEXT: %[[M3:[A-Za-z0-9_]+]] = OpUMod %uint %[[M4]] %uint_3
// SPIRV-NEXT: %[[M2:[A-Za-z0-9_]+]] = OpUMod %uint %[[M3]] %uint_2
// SPIRV-NEXT: %{{[A-Za-z0-9_]+}} = OpUMod %uint %[[M2]] %uint_1
// SPIRV-NOT: OpUDiv %uint %{{[A-Za-z0-9_]+}} %uint_0
// SPIRV-NOT: OpUMod %uint %{{[A-Za-z0-9_]+}} %uint_0

// HLSL-NOT: error[E
// HLSL: return {{.*}} / 4U / 3U / 2U;
// HLSL: return {{.*}} % 4U % 3U % 2U % 1U;
// HLSL-NOT: / 0U
// HLSL-NOT: % 0U

//TEST_INPUT:ubuffer(data=[100 10 23 7], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// The repro from the issue.
uint divideDown(uint rest)
{
    [ForceUnroll]
    for (uint i = 0u; i < 4u; ++i)
        rest /= 4u - i;
    return rest;
}

// The `%=` variant.
uint remainderDown(uint rest)
{
    [ForceUnroll]
    for (uint i = 0u; i < 4u; ++i)
        rest %= 4u - i;
    return rest;
}

// Digit d (0-3) under the k-th (0-23, factorial-base) permutation of four values.
uint permute4(uint d, uint k)
{
    uint avail = 0xe4u; // 0, 1, 2, 3 in 2-bit fields
    uint rest = k;
    uint result = 0u;
    [ForceUnroll]
    for (uint i = 0u; i < 4u; ++i)
    {
        let radix = 4u - i;
        let c = rest % radix;
        rest /= radix;
        if (i == d)
            result = (avail >> (2u * c)) & 3u;
        avail = (avail & ((1u << (2u * c)) - 1u)) | ((avail >> (2u * c + 2u)) << (2u * c));
    }
    return result;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // 100 / 4 / 3 / 2 / 1 = 4
    outputBuffer[0] = divideDown(inputBuffer[0]);
    // 10 % 4 % 3 % 2 % 1 = 0
    outputBuffer[1] = remainderDown(inputBuffer[1]);
    // Permutation 23 of (0, 1, 2, 3) is (3, 2, 1, 0).
    for (uint d = 0u; d < 4u; ++d)
        outputBuffer[2 + d] = permute4(d, inputBuffer[2]);
    // Permutation 7 of (0, 1, 2, 3) is (3, 1, 0, 2).
    for (uint d = 0u; d < 4u; ++d)
        outputBuffer[6 + d] = permute4(d, inputBuffer[3]);
}

// BUF: 4
// BUF-NEXT: 0
// BUF-NEXT: 3
// BUF-NEXT: 2
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 3
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 2

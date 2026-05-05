// Regression coverage for the non-`IRVar` `legalizeDefUse` path.
//
// This test returns a struct-typed expression directly from inside loop
// control flow, so legalization needs to handle a non-`IRVar` value that
// crosses multi-level break/continue rewrites.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):  -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=IR_BEFORE): -target spirv-asm -entry computeMain -stage compute -dump-ir-before removeRawDefaultConstructors
//TEST:SIMPLE(filecheck=IR_AFTER):  -target spirv-asm -entry computeMain -stage compute -dump-ir-after removeRawDefaultConstructors
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-mtl -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -compute -entry computeMain -output-using-type -profile cs_6_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -compute -entry computeMain -output-using-type

static const int N = 8;

struct OutVec
{
    float data[N];
};

uniform RWStructuredBuffer<float> outBuf;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outBuf

[ForceInline]
OutVec makeBagForSeed(int seed)
{
    OutVec output;

    [ForceUnroll]
    for (int i = 0; i < N; i++)
        output.data[i] = float(seed + i);

    return output;
}

[ForceInline]
OutVec chooseBagNonVar(int seed)
{
    for (int i = 0; i < 2; i++)
    {
        if (i == 0)
            continue;

        // Return an expression directly so legalization handles the non-var
        // value across transformed control flow.
        return makeBagForSeed(seed + i);
    }

    return makeBagForSeed(seed);
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    if (tid != 0)
        return;

    let result = chooseBagNonVar(0);
    for (int i = 0; i < N; i++)
        outBuf[i] = result.data[i];

    // BUF: 1.0
    // BUF-NEXT: 2.0
    // BUF-NEXT: 3.0
    // BUF-NEXT: 4.0
    // BUF-NEXT: 5.0
    // BUF-NEXT: 6.0
    // BUF-NEXT: 7.0
    // BUF-NEXT: 8.0
}

// HLSL:     void computeMain
// HLSL-NOT: {{0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?,[[:space:]]*0(\.0)?f?}}

// SPV:     OpEntryPoint
// SPV-NOT: OpConstantComposite {{.*}} %float_0 %float_0 %float_0 %float_0 %float_0 %float_0 %float_0 %float_0

// IR_BEFORE: %OutVec{{.*}}= defaultConstruct
// IR_AFTER-NOT: defaultConstruct

// Regression coverage for primitive-typed `legalizeDefUse` hoists.
//
// This test drives the non-`IRVar` path in `legalizeDefUse` with a primitive
// `float` value produced inside loop control flow that gets rewritten by
// `eliminateMultiLevelBreak`. The path seeds a temp var with raw
// `kIROp_DefaultConstruct`, which `removeRawDefaultConstructors` must strip.

//TEST:SIMPLE(filecheck=IR_BEFORE): -target spirv-asm -entry computeMain -stage compute -dump-ir-before removeRawDefaultConstructors
//TEST:SIMPLE(filecheck=IR_AFTER):  -target spirv-asm -entry computeMain -stage compute -dump-ir-after removeRawDefaultConstructors
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-mtl -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -compute -entry computeMain -output-using-type -profile cs_6_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -compute -entry computeMain -output-using-type

uniform RWStructuredBuffer<float> outBuf;
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outBuf

[ForceInline]
float chooseScalarNonVar(int seed)
{
    for (int i = 0; i < 2; ++i)
    {
        if (i == 0)
            continue;

        // Return expression (not local var) to exercise non-`IRVar` legalization.
        return float(seed + i);
    }

    return float(seed);
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    if (tid != 0)
        return;

    let result = chooseScalarNonVar(4);
    outBuf[0] = result;

    // BUF: 5.0
}

// IR_BEFORE: defaultConstruct
// IR_AFTER-NOT: defaultConstruct

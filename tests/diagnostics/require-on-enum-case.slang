// Regression test for https://github.com/shader-slang/slang/issues/14664.
//
// [require(...)] on an individual enum case must parse, and a consumer of
// the case must inherit that case's per-target capability requirements,
// mirroring whole-enum [require] behavior.
//
// The first invocation consumes the spirv-gated case for `-target hlsl` and
// must fail with E36107. The second consumes the hlsl-gated case for
// `-target hlsl` and must compile clean (exit-code=0, exhaustive: any
// diagnostic fails the test). The third is the control case: a whole-enum
// [require] consumed in the same position must keep failing with E36107.
// The fourth mirrors the first on `-target spirv` and must compile clean.
// The fifth mirrors the second on `-target spirv` and must fail with E36107,
// closing the symmetric direction.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKSPIRV,non-exhaustive):-target hlsl -entry computeMainSpirv -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKHLSL,exit-code=0):-target hlsl -entry computeMainHlsl -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKWHOLE,non-exhaustive):-target hlsl -entry computeMainWholeEnum -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKSPIRVCLEAN,exit-code=0):-target spirv -entry computeMainSpirv -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKHLSLSPIRV,non-exhaustive):-target spirv -entry computeMainHlsl -stage compute

enum MyEnum
{
    [require(spirv)]
    FirstValue = 0,

    [require(hlsl)]
    SecondValue = 1,
};

[require(spirv)]
enum MyWholeEnum
{
    Value = 0,
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainSpirv(uint3 tid : SV_DispatchThreadID)
{
    MyEnum v = MyEnum.FirstValue;
    if (v == MyEnum.FirstValue)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainHlsl(uint3 tid : SV_DispatchThreadID)
{
    MyEnum v = MyEnum.SecondValue;
    if (v == MyEnum.SecondValue)
    {
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainWholeEnum(uint3 tid : SV_DispatchThreadID)
{
    MyWholeEnum v = MyWholeEnum.Value;
    if (v == MyWholeEnum.Value)
    {
    }
}

//CHECKSPIRV: E36107
//CHECKWHOLE: E36107
//CHECKHLSLSPIRV: E36107

// Regression coverage for SLANG_ENABLE_SPIRV_OPT_MERGE_RETURN=OFF: the
// slang-glslang optimizer must still emit validator-clean SPIR-V when a helper
// has multiple return sites and InlineExhaustivePass is not registered.
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -emit-spirv-via-glsl -o multi-return-via-glsl.spv

RWStructuredBuffer<uint> result;

uint chooseValue(uint value)
{
    if (value == 0)
    {
        return 11;
    }

    if ((value & 1) != 0)
    {
        return value + 7;
    }

    return value + 13;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    result[0] = chooseValue(dispatchThreadID.x);
}

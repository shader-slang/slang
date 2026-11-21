// A runtime component index that falls in the padding range of a narrower element type
// (index 3 on a float3 texel) must not produce a select for the padding lane: only the
// element type's own three components are selectable, and the fourth lane of the
// 4-wide texel keeps the loaded value.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute

cbuffer Params
{
    uint channel;
}

RWTexture2D<float3> outputTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputTexture[tid.xy][channel + 1] = 0.25;

    //SPIRV-COUNT-3: OpSelect
    //SPIRV-NOT: OpSelect
    //GLSL: imageStore({{.*}}== 2U ?
    //GLSL-NOT: == 3U
}

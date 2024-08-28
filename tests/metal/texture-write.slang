//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal -DEMIT_SOURCE
//TEST:SIMPLE(filecheck=METALLIB): -stage compute -entry computeMain -target metallib -DEMIT_SOURCE -DMETALLIB
//TEST:SIMPLE(filecheck=HLSL): -stage compute -entry computeMain -target hlsl -DEMIT_SOURCE

// for some reason, metal textures dont have an overload for less-than-four component
// writes, they need to be converted to 4-components in a legalize step, as the other components
// get discarded
struct TextureWrite
{
    RWTexture2D<float> tex1;
    RWTexture2D<float2> tex2;
    RWTexture2D<float3> tex3;
    RWTexture2D<float4> tex4;

    RWTexture2DArray<float> tex1Array;
    RWTexture2DArray<float2> tex2Array;
    RWTexture2DArray<float3> tex3Array;
    RWTexture2DArray<float4> tex4Array;
}
ParameterBlock<TextureWrite> pWrites;

[numthreads(1, 1, 1)]
void computeMain()
{
    //TODO: add actual checks
    pWrites.tex1[uint2(0, 0)] = 0;
    pWrites.tex2[uint2(0, 0)] = float2(0, 0);
    pWrites.tex3[uint2(0, 0)] = float3(0, 0, 0);
    pWrites.tex4[uint2(0, 0)] = float4(0, 0, 0, 0);

    pWrites.tex1Array[uint3(0, 0, 0)] = 0;
    pWrites.tex2Array[uint3(0, 0, 0)] = float2(0, 0);
    pWrites.tex3Array[uint3(0, 0, 0)] = float3(0, 0, 0);
    pWrites.tex4Array[uint3(0, 0, 0)] = float4(0, 0, 0, 0);
}
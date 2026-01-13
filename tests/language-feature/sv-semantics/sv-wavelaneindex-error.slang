//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage hull -entry main

// CHECK: error 40015

// SV_WaveLaneIndex should work in all stages, but let's verify
// it errors when used as output (it's input-only)

struct HSOutput
{
    float4 pos : SV_Position;
};

struct HSConstOutput
{
    float edges[3] : SV_TessFactor;
    uint waveIndex : SV_WaveLaneIndex;  // ERROR: can't be output
};

HSConstOutput HSConst()
{
    HSConstOutput output;
    output.edges[0] = 1.0;
    output.edges[1] = 1.0;
    output.edges[2] = 1.0;
    output.waveIndex = 0;
    return output;
}

[shader("hull")]
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSConst")]
HSOutput main(
    InputPatch<float4, 3> input : SV_Position,
    uint id : SV_OutputControlPointID)
{
    HSOutput output;
    output.pos = input[id];
    return output;
}


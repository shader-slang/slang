// tests/wgsl/entry-point-legalization.slang
//TEST:SIMPLE(filecheck=CHK):-target wgsl-spirv-asm -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=CHK):-target wgsl-spirv-asm -stage vertex -entry vertexMain -DREPRODUCE

//CHK: %vertexMain = OpFunction

struct AssembledVertex
{
        float3  position;
};

struct VertexStageInput
{
    AssembledVertex assembledVertex : A;
};

struct VertexStageOutput
{
        float4                  sv_position             : SV_Position;
};

float3 GetPosition(VertexStageInput i)
{
    return i.assembledVertex.position;
}

[shader("vertex")]
VertexStageOutput vertexMain(VertexStageInput input)
{
        VertexStageOutput output;

#if defined(REPRODUCE)
        float3 position = GetPosition(input);
#else
        float3 position = input.assembledVertex.position;
#endif

        output.sv_position = float4(position, 1.0);
        return output;
}

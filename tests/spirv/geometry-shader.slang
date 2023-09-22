//TEST:SIMPLE(filecheck=CHECK):-entry gsMain -stage geometry -target spirv -emit-spirv-directly

struct GsOut
{
    float2 uv                               : TexCoord;
    float2 texelPos                         : TexelPosition;
    float4 posH                             : SV_Position;
};

// CHECK: OpEntryPoint

[maxvertexcount(15)]
void gsMain(uint triIdx : SV_PrimitiveID, inout TriangleStream<GsOut> outStream)
{
    for (int j = 0; j < 5; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            GsOut v;
            v.uv = float2(j,i);
            v.texelPos = float2(0,0);
            v.posH = float4(j, i, 1, 1);
            outStream.Append(v);
        }
        outStream.RestartStrip();
    }
}
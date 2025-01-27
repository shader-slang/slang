//TEST:SIMPLE(filecheck=CHECK): -target dxil -entry main -profile gs_6_0
struct GS_INPUT
{
    float4 PosSS : TEXTURE0;     // [Screen Space] Position
};

struct PS_INPUT
{
    float4 PosSS : SV_POSITION;  // [Screen Space] Position
};

[maxvertexcount(3)] 
void main(triangle GS_INPUT input[3], inout TriangleStream<PS_INPUT> outStream)
{
    PS_INPUT output;

    output.PosSS = input[0].PosSS;
    outStream.Append(output);
     
    output.PosSS = input[1].PosSS;
    outStream.Append(output);

    output.PosSS = input[2].PosSS;
    outStream.Append(output);

    outStream.RestartStrip();
}
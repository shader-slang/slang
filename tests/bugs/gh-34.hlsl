//TEST:COMPARE_HLSL: -profile gs_5_0 -target dxbc-assembly -no-checking

struct VS_OUT { float3 p : POSITION; };

[maxvertexcount(3)]
void main(InputPatch<VS_OUT, 3> input, inout TriangleStream<VS_OUT> outStream)
{
    VS_OUT output;
    for (uint i = 0; i < 3; i += 1)
    {
        output = input[i];
        outStream.Append(output);
    }

    outStream.RestartStrip();
}

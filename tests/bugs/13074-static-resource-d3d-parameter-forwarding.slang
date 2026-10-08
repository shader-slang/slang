//TEST:SIMPLE(filecheck=CHECK): -O0 -target hlsl -entry main -stage compute -validate-ir

// HLSL can forward a by-value resource parameter through ordinary function calls. The D3D
// resource-input inlining step considers a call only when an argument has neither a form accepted
// by resource specialization nor a direct origin in the caller's parameters. This test passes a
// supported global resource, so both call boundaries must remain intact.

Texture2D<float4> inputTexture;
RWStructuredBuffer<float4> output;

[noinline]
float4 readTexture(Texture2D<float4> texture)
{
    return texture.Load(int3(0, 0, 0));
}

[noinline]
float4 forwardTexture(Texture2D<float4> texture)
{
    return readTexture(texture);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = forwardTexture(inputTexture);
}

//CHECK-LABEL: float4 readTexture_
//CHECK-LABEL: float4 forwardTexture_
//CHECK: return readTexture_{{[0-9]+}}(texture_
//CHECK-LABEL: void main(
//CHECK: forwardTexture_{{[0-9]+}}(inputTexture_

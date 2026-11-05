//TEST:SIMPLE(filecheck=CHECK): -O0 -target hlsl -entry main -stage compute -validate-ir

// Generated HLSL can pass `inputTexture` to `forwardTexture`, which can in turn pass its parameter
// to `readTexture`. Neither argument is loaded from mutable local resource storage, so the D3D
// resource-input inlining in `specializeResourceUsage` must leave both call boundaries intact. The
// checks pin both definitions and both calls.

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

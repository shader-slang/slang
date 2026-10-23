//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target hlsl -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target spirv -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target metal -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target cpp -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target hlsl -entry main -validate-ir -no-mangle -DREAD_WHOLE_BUFFER
//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target spirv -entry main -validate-ir -no-mangle -DREAD_WHOLE_BUFFER

// Compatibility creates mutable shadows for direct resource values and fixed-size resource arrays.
// Resource-global legalization must replace those shadows before emission. Explicit parameter
// groups and legacy buffers containing resources remain read-only aliases.
Texture2D<float4> gTex;
Texture2D<float4> textures[2];
SamplerState sampler;
struct Data { float4 value; };
ConstantBuffer<Data> explicitBuffer;
cbuffer Mixed
{
    Texture2D<float4> mixedTexture;
    float4 mixedValue;
};
RWStructuredBuffer<float4> output;

#ifdef READ_WHOLE_BUFFER
// A whole-buffer read must produce the element struct, just as a read of its fields does.
// The `-no-mangle` option exposes the legacy buffer variable under the name `Mixed`.
T copyValue<T>(T value)
{
    return value;
}

// Overload resolution would select this function if `Mixed` retained its `ConstantBuffer`
// type. Its scalar result would make the subsequent field accesses invalid.
uint copyValue<T>(ConstantBuffer<T> value)
{
    return 0;
}
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = gTex.SampleLevel(sampler, float2(0), 0) +
        textures[1].Load(int3(0)) + explicitBuffer.value +
        mixedTexture.Load(int3(0)) + mixedValue;
#ifdef READ_WHOLE_BUFFER
    output[tid.x] += copyValue(Mixed).mixedTexture.Load(int3(0)) + copyValue(Mixed).mixedValue;
#endif
}

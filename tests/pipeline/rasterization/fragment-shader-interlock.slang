// fragment-shader-interlock.slang

// Test that explicit use of fragment-shader interlock (FSI)
// operations is supported by Slang

//TEST:CROSS_COMPILE:-target spirv -entry main -stage fragment

[shader("fragment")]
void main(
    float4 coords : COORDS,
    uniform RasterizerOrderedTexture2D<float4> texture,
    out float4 result : SV_Target)
{
    beginInvocationInterlock();

    result = texture[uint2(coords.xy)];
    texture[uint2(coords.xy)] = result + coords;

    endInvocationInterlock();
}

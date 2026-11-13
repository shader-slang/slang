// Check that reading a texture from a zero-length array compiles to SPIR-V without an `OpUndef` of
// the image type. The element of a zero-length array is a `Poison` texture, which lives at module
// scope rather than in the function that reads it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute

// CHECK: %[[IMG:[A-Za-z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1
// CHECK-NOT: OpUndef %[[IMG]]

Texture2D<float4> gTexture;
Texture2D<float4> gNoTextures[0];
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    outputBuffer[0] = gNoTextures[id.x].Load(int3(0)) + gTexture.Load(int3(0));
}

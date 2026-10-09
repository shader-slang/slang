// Recursion validation normally rejects this shader before resource specialization. Disabling that
// validation exercises the D3D resource-input inlining step itself. The entry-point call passes a
// resource loaded from generated local storage, so the step would otherwise try to inline it.
// Inlining a callee in the `readEven`/`readOdd` cycle would copy another call from that cycle into
// the entry point on every retry. The pass must instead preserve the cycle and terminate. The
// separate `__constref` use keeps the generated local addressable so that the late storage
// diagnostic provides a stable endpoint.

//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=CHECK): -O0 -target dxil -entry main -stage compute -validate-ir -disable-non-essential-validations

Texture2D<float4> textures[2];
RWStructuredBuffer<float4> output;
static Texture2D<float4> selectedTexture = textures[0];
//CHECK:                 ^^^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//CHECK:                 ^^^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'Texture2D', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable

[noinline]
float4 readEven(Texture2D<float4> texture, uint depth)
{
    if (depth == 0)
        return texture.Load(int3(0, 0, 0));
    return readOdd(texture, depth - 1);
}

[noinline]
float4 readOdd(Texture2D<float4> texture, uint depth)
{
    if (depth == 0)
        return texture.Load(int3(0, 0, 0));
    return readEven(texture, depth - 1);
}

[noinline]
float4 readAddressed(__constref Texture2D<float4> texture)
{
    return texture.Load(int3(0, 0, 0));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    selectedTexture = textures[threadID.x & 1];
    output[threadID.x] = readEven(selectedTexture, threadID.x) +
                         readAddressed(selectedTexture);
}

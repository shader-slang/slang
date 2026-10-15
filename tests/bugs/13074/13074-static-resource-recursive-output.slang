// Recursion validation normally rejects this shader before resource specialization. Disabling that
// validation lets the resource-return path see a recursive function. The constant-buffer argument
// also makes input specialization create a new recursive clone while this pass is running. Output
// specialization must recognize the new cycle instead of repeatedly expanding it. The separate
// `__constref` use keeps the generated local addressable so that the late storage diagnostic
// provides a stable endpoint after the pass terminates.

//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=CHECK): -O0 -target dxil -entry main -stage compute -validate-ir -disable-non-essential-validations

Texture2D<float4> textures[2];
RWStructuredBuffer<float4> output;
struct Options
{
    uint depth;
};
ConstantBuffer<Options> options;
static Texture2D<float4> selectedTexture = textures[0];
//CHECK:                 ^^^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//CHECK:                 ^^^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'Texture2D', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable

[noinline]
Texture2D<float4> selectRecursively(ConstantBuffer<Options> callOptions, uint depth)
{
    if (depth == 0)
        return selectedTexture;
    return selectRecursively(callOptions, depth - 1);
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
    Texture2D<float4> selected = selectRecursively(options, options.depth);
    output[threadID.x] = selected.Load(int3(0, 0, 0)) + readAddressed(selectedTexture);
}

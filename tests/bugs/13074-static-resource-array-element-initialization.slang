//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -entry main -stage fragment -validate-ir

Texture2D<float4> textures[2];
static Texture2D<float4> cachedTextures[2];
//CHECK:                 ^^^^^^^^^^^^^^ cannot prove complete initialization of a resource array declared static at file or namespace scope
//CHECK:                 ^^^^^^^^^^^^^^ resource array 'cachedTextures' is declared static at file or namespace scope, may be written one element at a time, and may be read before a whole-array assignment; assign the whole array before reading it

// Assigning every element initializes this particular array, but resource-global legalization does
// not track a separate definite-assignment fact for each index. We reject the element-wise form
// until the pass can prove that every path assigns every element.

[noinline]
void initializeCachedTextures()
{
    cachedTextures[0] = textures[0];
    cachedTextures[1] = textures[1];
}

[shader("fragment")]
float4 main() : SV_Target
{
    initializeCachedTextures();
    return cachedTextures[0].Load(int3(0, 0, 0)) +
           cachedTextures[1].Load(int3(0, 0, 0));
}

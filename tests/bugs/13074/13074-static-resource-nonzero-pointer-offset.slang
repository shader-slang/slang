//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry main -stage compute -validate-ir

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTextures[2][1];

[numthreads(1, 1, 1)]
void main()
{
    Texture2D<float4> textures[1] = {inputTexture};
    var pointerToArray = &cachedTextures[0];
    pointerToArray[1] = textures;
//CHECK:              ^ address use may observe the identity of mutable resource storage
//CHECK:              ^ the program may retain the address of mutable resource variable 'cachedTextures' or observe its storage identity; replacing the variable with function-local storage might change that behavior
}

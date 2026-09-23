// Indexing a pointer to the array at offset zero still points to the complete array. The resulting
// `GetOffsetPtr` store therefore initializes the whole `cachedTextures` value rather than one array
// element.

//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -validate-ir -o 13074-static-resource-array-whole-write-through-zero-offset.cpp

RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTextures[1];

[noinline]
void initializeCachedTextures(Texture2D<float4> textures[1])
{
    var pointerToArray = &cachedTextures;
    pointerToArray[0] = textures;
}

[numthreads(1, 1, 1)]
void computeMain(Texture2D<float4> texture)
{
    Texture2D<float4> textures[1] = {texture};
    initializeCachedTextures(textures);
    output[0] = cachedTextures[0].Load(int3(0, 0, 0));
}

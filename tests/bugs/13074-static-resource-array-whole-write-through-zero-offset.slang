// Indexing a pointer to the array at offset zero still points to the complete array. The resulting
// `GetOffsetPtr` store therefore initializes the whole `cachedTextures` value rather than one array
// element.

//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -validate-ir -o 13074-static-resource-array-whole-write-through-zero-offset.cpp
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -entry computeMain -shaderobj -output-using-type

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000

//TEST_INPUT: Texture2D(size=4, content = one):name=sourceTexture
Texture2D<float4> sourceTexture;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=16):out,name=output
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTextures[1];

[noinline]
void initializeCachedTextures(Texture2D<float4> textures[1])
{
    var pointerToArray = &cachedTextures;
    pointerToArray[0] = textures;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Texture2D<float4> textures[1] = {sourceTexture};
    initializeCachedTextures(textures);
    output[0] = cachedTextures[0].Load(int3(0, 0, 0));
}

//TEST:SIMPLE: -target spirv -entry main -stage fragment -o 13074-static-resource-array.spv
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment

//CHECK: OpName %textures "textures"
//CHECK-NOT: unusedImage
//CHECK: %[[ADDRESS:[A-Za-z0-9_]+]] = OpAccessChain {{.*}} %textures %int_0
//CHECK: %[[TEXTURE:[A-Za-z0-9_]+]] = OpLoad {{.*}} %[[ADDRESS]]
//CHECK: OpImageFetch {{.*}} %[[TEXTURE]]

Texture2D<float4> textures[1];
static RWTexture2D<float4> unusedImage;
static Texture2D<float4> cachedTextures[1] = textures;

[noinline]
float4 loadCachedTexture()
{
    return cachedTextures[0].Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    return loadCachedTexture();
}

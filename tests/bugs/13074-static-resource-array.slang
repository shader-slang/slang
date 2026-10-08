// `unusedImage` proves that `legalizeResourceGlobalVars` removes a selected global with no uses.
// The one-element `cachedTextures` array is initialized from the `textures` array; the pass must
// replace that global and pass the array value through `loadCachedTexture`. The SPIR-V checks prove
// that neither static resource remains and that the helper ultimately loads the original shader
// parameter's first element.

//TEST:SIMPLE: -target spirv -entry main -stage fragment -o 13074-static-resource-array.spv
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment
//TEST:SIMPLE(filecheck=NOSTATIC): -target spirv-asm -entry main -stage fragment

//CHECK: OpName %textures "textures"
//NOSTATIC-NOT: unusedImage
//NOSTATIC-NOT: cachedTextures
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

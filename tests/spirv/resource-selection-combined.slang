//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry precombined -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry precombined -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry precombined -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry mixed -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry mixed -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PROJECTED): -target spirv-asm -stage fragment -entry projected -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PROJECTED): -target spirv-asm -stage fragment -entry projected -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry constructedSampler -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry constructedSampler -validate-ir -validate-ir-detailed

// A combined texture and sampler can come from a single declared binding or from a texture
// and a sampler combined in the program. We preserve each complete choice through selection;
// a single combined binding cannot be replaced with imaginary separate bindings.
// The explicit pair constructor is the SPIR-V library intrinsic. GLSL library methods construct
// texture/sampler pairs through textual intrinsics, which are covered by the other test files.
// SPIRV: OpImageSampleExplicitLod
// GLSL: textureLod

Sampler2D<float4> combinedA;
Sampler2D<float4> combinedB;
Texture2D<float4> texture;
SamplerState sampler;
Texture2D<float4> textureArray[4];
SamplerState otherSampler;
cbuffer Parameters { uint index; uint otherIndex; };

float4 precombined(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = select(uv.x > 0.5, combinedA, combinedB);
    return selected.SampleLevel(uv, 0);
}

float4 mixed(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = select(uv.x > 0.5,
        combinedA, __makeCombinedTextureSampler(texture, sampler));
    return selected.SampleLevel(uv, 0);
}

// `Load` extracts the texture from a combined texture and sampler before fetching a texel.
// Both incoming textures use `textureArray`, so discarding the sampler must merge the two
// texture indices rather than discard one index together with the sampler distinction.
// PROJECTED-DAG: OpAccessChain {{.*}} %textureArray %[[INDEX:[^ ]+]]
// PROJECTED-DAG: %[[INDEX]] = OpSelect %uint
// PROJECTED: OpImageFetch
// PROJECTED-NOT: OpImageFetch
float4 projected(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = select(uv.x > 0.5,
        __makeCombinedTextureSampler(textureArray[index], sampler),
        __makeCombinedTextureSampler(textureArray[otherIndex], otherSampler));
    return selected.Load(int3(0));
}

// Extracting the sampler from a pair constructed in the program is valid: the two sampler
// arguments already exist as separate resources. We require projection to preserve their
// conditional choice and remove the pair construction before the sample is emitted.
float4 constructedSampler(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = select(uv.x > 0.5,
        __makeCombinedTextureSampler(texture, sampler),
        __makeCombinedTextureSampler(texture, otherSampler));
    return texture.SampleLevel(selected.__getSampler(), uv, 0);
}

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target spirv-asm -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target glsl -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target glsl -stage fragment -entry imageFromBinding -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target glsl -stage fragment -entry constructedPair -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=IMAGE): -target spirv-asm -stage fragment -entry imageFromBinding -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=IMAGE): -target spirv-asm -stage fragment -entry imageFromBinding -validate-ir -validate-ir-detailed -O1

// A precombined binding does not expose a separately addressable sampler on Khronos targets.
// `specializeResourceSelections` must leave conditional resource operations intact when recreating
// their possible values would need that extraction.
// In contrast, a texture and sampler combined by the program can be separated by retaining the
// actual sampler value; `resource-selection-combined.slang` covers that sampler extraction.
// CHECK: error[E56004]: opaque type in local variable
// IMAGE: %[[EXTRACTED:[^ ]+]] = OpImage {{[^ ]+}}
// IMAGE: OpImageFetch {{[^ ]+}} %[[EXTRACTED]]{{( |$)}}

Sampler2D<float4> combinedA;
Sampler2D<float4> combinedB;
Texture2D<float4> texture;
SamplerState ordinarySampler;

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = uv.x > 0.5 ? combinedA : combinedB;
    SamplerState sampler = selected.__getSampler();
    return texture.SampleLevel(sampler, uv, 0);
}

// SPIR-V can extract an image with `OpImage`. The GLSL path has no standalone image-extraction
// emitter; its normal combined-texture methods use target intrinsics directly instead.
float4 imageFromBinding(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = uv.x > 0.5 ? combinedA : combinedB;
    return selected.__getTexture().Load(int3(0));
}

// The internal constructor is used by the SPIR-V standard-library path. Ordinary GLSL sampling
// uses textual target intrinsics, so an explicit constructor must not be reconstructed there.
float4 constructedPair(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = uv.x > 0.5
        ? combinedA
        : __makeCombinedTextureSampler(texture, ordinarySampler);
    return selected.SampleLevel(uv, 0);
}

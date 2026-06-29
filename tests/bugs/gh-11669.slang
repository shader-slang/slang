//TEST:SIMPLE(filecheck=WGSL):  -target wgsl  -entry main_fragment -stage fragment
//TEST:SIMPLE(filecheck=METAL): -target metal -entry main_fragment -stage fragment

// shader-slang/slang#11669: GetDimensions on a combined texture-sampler must query the texture
// only. On targets that lower the combined `Sampler2D` value into a {texture, sampler} pair, the
// lowered call gains an injected sampler operand at index 1. The positional `$N` accessors in the
// GetDimensions intrinsic string previously addressed that sampler, so the texture width was
// written through the sampler global and every output argument was shifted by one (the second
// output was never written). The `$X` skip-sampler marker makes the texture-only query skip the
// injected sampler so the outputs line up again.
//
// WGSL and Metal are asserted below. CUDA uses the same `$X`-prefixed positional string and is
// fixed by construction (it is not separately asserted here). HLSL/CPU emit GetDimensions via the
// leading-`.` member form, which does not route through `$N` expansion; fixing those needs a
// separate rewrite to an explicit string and is tracked as the #10522 family.

Sampler2D<float4> input_texture;

[shader("pixel")]
float4 main_fragment() : SV_Target
{
    // The texture extents must be read from the texture, and must not be written through the
    // sampler global (the bug stored the width into the sampler).
    // WGSL: textureDimensions(
    // WGSL-NOT: sampler{{[0-9_]*}})) =

    // METAL: get_width(
    // METAL-NOT: sampler{{[0-9_]*}})) =

    float2 dimensions;
    input_texture.GetDimensions(dimensions.x, dimensions.y);
    return float4(dimensions, 0, 1);
}

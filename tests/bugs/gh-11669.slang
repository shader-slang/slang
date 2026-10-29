//TEST:SIMPLE(filecheck=WGSL):  -target wgsl  -entry main_fragment -stage fragment
//TEST:SIMPLE(filecheck=METAL): -target metal -entry main_fragment -stage fragment

// shader-slang/slang#11669: GetDimensions on a combined texture-sampler must query the texture
// only. On targets that lower the combined `Sampler2D` value into a {texture, sampler} pair, the
// lowered call gains an injected sampler operand at index 1. The positional `$N` accessors in the
// GetDimensions intrinsic string previously addressed that sampler, so the texture width was
// written through the sampler global and every output argument was shifted by one (the second
// output was never written). The WGSL string starts with the `$q` marker, which skips the
// injected sampler so the outputs line up. Metal forwards the combined case to the extracted
// texture, as HLSL and CPU do.
//
// WGSL and Metal both lower the combined value and emit GetDimensions through positional `$N`
// strings, so both are asserted below. CUDA is not in the combined-sampler lowering set (its
// `Sampler2D` stays a single `CUtexObject`), so it needs no marker.
// `tests/bugs/gh-13737.slang` covers HLSL, `tests/bugs/gh-13737-cpu.slang` covers CPU, and
// `tests/bugs/gh-14007.slang` covers Metal argument buffers.

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

// Regression for #11874: at `-profile glsl_450`, calling texelFetch-style
// `Load` on a *combined* `Sampler2D<uint4>` used to emit a spurious
// `warning[E41012]: profile implicitly upgraded ... GL_EXT_samplerless_texture_functions`,
// even though the emitted GLSL uses `texelFetch(usampler2D, ...)` and never
// references that extension.
//
// The readonly `Load`/subscript methods (source/slang/hlsl.meta.slang) carried
// an unconditional static `[require(..., texture_sm_4_1_samplerless)]`, which
// lowers to an `IRLateRequireCapability` inst that drives E41012 for every
// instantiation -- that static path cannot see the generic `isCombined` gate.
// The `#extension` line, by contrast, is (correctly) emitted only for *bare*
// textures via an `isCombined`-gated `__requireTargetExtension`. Dropping the
// samplerless atom from the static require aligns `Load` with `GetDimensions`,
// which already relies solely on the emit-time mechanism.

// Combined sampler: no warning, and no `#extension` (uses texelFetch(usampler2D)).
//TEST:SIMPLE(filecheck=COMBINED): -target glsl -profile glsl_450 -stage vertex -entry combinedMain
// COMBINED-NOT: warning[E41012]
// COMBINED-NOT: GL_EXT_samplerless_texture_functions
// COMBINED: texelFetch

// Bare texture: the `#extension` is still emitted at code-gen time (unchanged
// by the fix), and no warning is produced (matching GetDimensions).
//TEST:SIMPLE(filecheck=BARE): -target glsl -profile glsl_450 -stage vertex -entry bareMain
// BARE-NOT: warning[E41012]
// BARE: #extension GL_EXT_samplerless_texture_functions
// BARE: texelFetch

uniform Sampler2D<uint4> combined;
uniform Texture2D<uint4> bare;

[shader("vertex")]
float4 combinedMain() : SV_Position
{
    return float4(combined.Load(int3(1, 2, 0)));
}

[shader("vertex")]
float4 bareMain() : SV_Position
{
    return float4(bare.Load(int3(1, 2, 0)));
}

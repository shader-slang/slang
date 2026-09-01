// Regression for #11874: at `-profile glsl_450`, calling a texelFetch-style
// `Load` on a *combined* `Sampler2D<uint4>` used to emit a spurious
// `warning[E41012]: profile implicitly upgraded ... GL_EXT_samplerless_texture_functions`,
// even though the emitted GLSL uses `texelFetch(usampler2D, ...)` and never
// references that extension.
//
// Consider the `combinedMain` entry point below. The readonly `Load`/subscript
// methods on `_Texture` (source/slang/hlsl.meta.slang) carried an unconditional
// static `[require(..., texture_sm_4_1_samplerless)]`, where
// `texture_sm_4_1_samplerless = texture_sm_4_1 | GL_EXT_samplerless_texture_functions`.
// Because that attribute is attached to *every* instantiation of the method --
// regardless of the generic `isCombined` value -- the samplerless extension is
// pulled into the entry point's inferred capability set. The AST-level check in
// `checkEntryPointName`/`ProfileImplicitlyUpgraded` (source/slang/slang-check-shader.cpp,
// ~line 2244) then compares that inferred set against the requested `-profile` and,
// finding the extension missing, warns that the profile was implicitly upgraded
// (E41012). This check runs during semantic checking, before code emission, so it
// cannot observe the `isCombined`-gated `__requireTargetExtension(...)` in each
// method's `case glsl:` body -- which (correctly) emits the `#extension` line only
// for *bare* textures. Dropping the samplerless atom from the static require aligns
// `Load` with `GetDimensions`, which already relies solely on that emit-time
// mechanism and never warns spuriously.

// Combined sampler: no warning, and no `#extension` (uses texelFetch(usampler2D)).
//TEST:SIMPLE(filecheck=COMBINED): -target glsl -profile glsl_450 -stage vertex -entry combinedMain
// COMBINED-NOT: warning[E41012]
// COMBINED-NOT: GL_EXT_samplerless_texture_functions
// COMBINED: texelFetch

// Bare texture: the `#extension` is still emitted at code-gen time (unchanged by
// the fix), and no warning is produced (matching GetDimensions).
//TEST:SIMPLE(filecheck=BARE): -target glsl -profile glsl_450 -stage vertex -entry bareMain
// BARE-NOT: warning[E41012]
// BARE: #extension GL_EXT_samplerless_texture_functions
// BARE: texelFetch

// Combined multisampled sampler: same as the non-MS combined case (no warning,
// no `#extension`). Exercises the multisample `Load(location, sampleIndex)` sites.
//TEST:SIMPLE(filecheck=COMBINED_MS): -target glsl -profile glsl_450 -stage vertex -entry combinedMSMain
// COMBINED_MS-NOT: warning[E41012]
// COMBINED_MS-NOT: GL_EXT_samplerless_texture_functions
// COMBINED_MS: texelFetch

// Bare multisampled texture: `#extension` still emitted, no warning.
//TEST:SIMPLE(filecheck=BARE_MS): -target glsl -profile glsl_450 -stage vertex -entry bareMSMain
// BARE_MS-NOT: warning[E41012]
// BARE_MS: #extension GL_EXT_samplerless_texture_functions
// BARE_MS: texelFetch

uniform Sampler2D<uint4> combined;
uniform Texture2D<uint4> bare;
uniform Sampler2DMS<uint4> combinedMS;
uniform Texture2DMS<uint4> bareMS;

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

[shader("vertex")]
float4 combinedMSMain() : SV_Position
{
    return float4(combinedMS.Load(int2(1, 2), 0));
}

[shader("vertex")]
float4 bareMSMain() : SV_Position
{
    return float4(bareMS.Load(int2(1, 2), 0));
}

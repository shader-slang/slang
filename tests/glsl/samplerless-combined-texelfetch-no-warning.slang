// Regression for #11874 (extended to the GLSL `texelFetch` wrappers): the combined-sampler
// `texelFetch`/`texelFetchOffset` GLSL-compat free functions (source/slang/glsl.meta.slang)
// carried an unconditional static `[require(..., texture_sm_4_1_samplerless)]`, so
// `texelFetch(mySampler2D, ...)` under an explicit `-profile` emitted the same spurious
// `warning[E41012]: profile implicitly upgraded ... GL_EXT_samplerless_texture_functions` as
// `.Load` did -- even though a *combined* sampler lowers to `texelFetch(sampler2D, ...)` and
// never needs that extension. These wrappers are combined-only (there is no bare form of a
// `Sampler1D`/`Sampler2DRect`/`SamplerBuffer`, and the generic `_Texture` overloads fix
// `isCombined=1`), so dropping the samplerless atom to plain `texture_sm_4_1` is correct; the
// extension is still added at emit time, for the bare case only, by `_Texture.Load`.

// texelFetch on a combined 2D sampler: no warning, no `#extension` (uses `texelFetch(sampler2D)`).
//TEST:SIMPLE(filecheck=TF2D): -allow-glsl -target glsl -profile glsl_450 -stage fragment -entry tf2d
// TF2D-NOT: warning[E41012]
// TF2D-NOT: GL_EXT_samplerless_texture_functions
// TF2D: texelFetch

// texelFetch on a combined multisample sampler (hits the `case glsl:` intrinsic path).
//TEST:SIMPLE(filecheck=TF2DMS): -allow-glsl -target glsl -profile glsl_450 -stage fragment -entry tf2dms
// TF2DMS-NOT: warning[E41012]
// TF2DMS-NOT: GL_EXT_samplerless_texture_functions
// TF2DMS: texelFetch

// texelFetchOffset on a combined 2D sampler.
//TEST:SIMPLE(filecheck=TFO2D): -allow-glsl -target glsl -profile glsl_450 -stage fragment -entry tfo2d
// TFO2D-NOT: warning[E41012]
// TFO2D-NOT: GL_EXT_samplerless_texture_functions
// TFO2D: texelFetch

uniform sampler2D s2d;
uniform sampler2DMS s2dms;

[shader("fragment")]
float4 tf2d() : SV_Target { return texelFetch(s2d, int2(1, 2), 0); }

[shader("fragment")]
float4 tf2dms() : SV_Target { return texelFetch(s2dms, int2(1, 2), 0); }

[shader("fragment")]
float4 tfo2d() : SV_Target { return texelFetchOffset(s2d, int2(1, 2), 0, int2(0, 0)); }

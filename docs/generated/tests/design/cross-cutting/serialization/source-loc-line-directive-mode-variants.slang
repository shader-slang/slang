//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#source-location-serialization
//META: doc_section_digest=0b9df2a75e79b7e8b6026192b354673f12b076885b33df7d9612c34bd2b3f441
//META: purpose=The emitted #line directives are the rendering of the retained source locations, not a fixed preamble: -line-directive-mode none drops them entirely while the glsl mode renders the same locations with a file number instead of a path.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The counterfactual for source-loc-line-directives-multi-target.slang.
// If the `#line` text there were boilerplate, suppressing it would be
// impossible and its file reference would be fixed. The NONE directive
// asserts the emitted HLSL still contains the kernel body but no #line
// token at all, and the GLSLNUM directive asserts the glsl mode renders
// the same locations as `#line <line> <fileNumber>` - a line number
// with a numeric file id and no quoted path. Together they show the
// directive content is derived per compile from the retained locations.
//
// Both halves are a property of the shared C-family line-directive
// writer rather than of one back-end, so the same pair is asserted on
// the Metal and WGSL emitters too: METALNONE / WGSLNONE assert the
// suppression, METALNUM / WGSLNUM assert the numeric-file rendering.
// WGSL is the interesting one - `#line` is not WGSL syntax and the
// target emits no directives by default, yet asking for the glsl mode
// still renders the retained locations, which is only possible if the
// locations themselves survived to emit. The SPIR-V emitter has no
// `#line` concept at all and ignores -line-directive-mode outright
// (its output is byte-identical under `none` and `standard`), so it is
// recorded as unsupported-on-target in the bundle README instead of
// being given a directive here.

//TEST:SIMPLE(filecheck=NONE):-target hlsl -entry main -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSLNUM):-target glsl -entry main -stage compute -line-directive-mode glsl
//TEST:SIMPLE(filecheck=METALNONE):-target metal -entry main -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=METALNUM):-target metal -entry main -stage compute -line-directive-mode glsl
//TEST:SIMPLE(filecheck=WGSLNONE):-target wgsl -entry main -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=WGSLNUM):-target wgsl -entry main -stage compute -line-directive-mode glsl

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 7;
}

// NONE-NOT: #line
// NONE: void main(
// NONE-NOT: #line

// GLSLNUM: #line {{[0-9]+}} {{[0-9]+}}

// METALNONE-NOT: #line
// METALNONE: void main_0(
// METALNONE-NOT: #line

// METALNUM: #line {{[0-9]+}} {{[0-9]+}}

// WGSLNONE-NOT: #line
// WGSLNONE: fn main(
// WGSLNONE-NOT: #line

// WGSLNUM: #line {{[0-9]+}} {{[0-9]+}}

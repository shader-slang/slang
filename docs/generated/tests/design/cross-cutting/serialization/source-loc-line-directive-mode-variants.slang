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

//TEST:SIMPLE(filecheck=NONE):-target hlsl -entry main -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSLNUM):-target glsl -entry main -stage compute -line-directive-mode glsl

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

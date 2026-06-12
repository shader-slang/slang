//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#parameter-group-constant-buffer-types
//META: doc_section_digest=a62b9719fec7c122df80f8f50c81b7a71a2f4deb2062c135375ac53227244677
//META: purpose=A ParameterBlockType<T> uniform is the AST type for `ParameterBlock<T>`; the HLSL backend lowers it through the parameter-group machinery to a cbuffer block of T's fields.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ParameterBlockType` is a UniformParameterGroupType leaf for the
// Slang-specific `ParameterBlock<T>`. The doc says the family
// inherits from `UniformParameterGroupType` so the carrier machinery
// is shared. Observed by HLSL emit: the fields of T appear as a
// cbuffer-like block.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Frame { float time; int frameIndex; }

ParameterBlock<Frame> g_frame;
RWStructuredBuffer<float> g_out;

//CHECK: cbuffer
//CHECK: time
//CHECK: frameIndex
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_frame.time * float(g_frame.frameIndex);
}

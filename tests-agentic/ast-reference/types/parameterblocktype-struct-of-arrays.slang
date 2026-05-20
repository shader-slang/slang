//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#parameter-group-constant-buffer-types
//META: doc_section_digest=a62b9719fec7c122df80f8f50c81b7a71a2f4deb2062c135375ac53227244677
//META: purpose=ParameterBlockType<T> with a struct-of-arrays payload still lowers through the parameter-group machinery to a cbuffer-like HLSL block.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the structured-payload edge of ParameterBlockType:
// the struct holds fixed-size arrays of two different scalar types.
// The HLSL emit still produces a cbuffer block, exercising the
// parameter-group machinery across non-trivial member types.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct SoA { float values[4]; int counts[4]; }

ParameterBlock<SoA> g_pb;
RWStructuredBuffer<float> g_out;

//CHECK: cbuffer
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_pb.values[0] + float(g_pb.counts[0]);
}

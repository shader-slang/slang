//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=9574aa361d7b18b1da022473e0e53acd215b9af06dae0cb39490ca13610d03bf
//META: purpose=A ParameterBlock is pointer-like for lookup too, so a field of its element type is reachable through the same auto-dereference step.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the auto-dereference on the second pointer-like resource
// shape, so the claim is exercised beyond ConstantBuffer alone.
// ParameterBlock<Params> has no member `a`; the access resolves only
// via the pointee. CHECK asserts the emitted HLSL renders the read as
// a field access on the block.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

struct Params { float a; }

ParameterBlock<Params> pb;

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    // CHECK: pb_{{[0-9]+}}.a_{{[0-9]+}}
    outBuf[0] = pb.a;
}

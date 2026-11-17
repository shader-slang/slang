//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=8a7a3c63487b8f293c70c075134ef33eae10ff00b2e4f1c4f9b195bed7528e0c
//META: purpose=Boundary: RWStructuredBuffer parameterised by a vector element type names the Vec(...) form as its first operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// RWStructuredBuffer parameterised by a vector element type names the
// Vec(...) form as its first operand. A `RWStructuredBuffer<float4>`
// surfaces the full `Vec(Float, 4 : Int)` as its first operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<float4> buf;
uniform float4 v;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v;
}

// IR: let {{.*}}%buf{{.*}}: RWStructuredBuffer(Vec(Float, 4 : Int){{.*}}= global_param

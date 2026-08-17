//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=4c5526c573a01002c5664c82dafecd90b68ec6487726865d6f3eb388eabf2bf3
//META: purpose=A ParameterBlock consumes a whole register space, recorded as LayoutResourceKind 12 in operand 0 of an offset attribute on its container var layout.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the register-space row of the resource-kind enumeration, which is
// the one kind that is a whole binding space rather than a slot inside one.
// It only appears on a target that binds by descriptor set, so the directive
// uses SPIR-V. The CHECKs walk from the named `gPB` field key through its var
// layout to the parameter-group record and then into that record's operand 0,
// the container var layout, which is where the space offset lives; the same
// dump also shows an ordinary slot offset of kind 9 further in, so the two
// kinds are visibly distinct rather than interchangeable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct Material
{
    Texture2D albedo;
    SamplerState samp;
    float4 tint;
}

ParameterBlock<Material> gPB;
RWStructuredBuffer<float> gOut;

[numthreads(1, 1, 1)]
void main()
{
    gOut[0] = gPB.tint.x;
}

// CHECK: = structFieldLayout(%gPB, %[[VL:[0-9]+]])
// CHECK: {{^}}let %[[VL]]{{[^0-9]}}{{.*}}= varLayout(%[[PGTL:[0-9]+]],
// CHECK: {{^}}let %[[PGTL]]{{[^0-9]}}{{.*}}= parameterGroupTypeLayout(%[[CONTAINER:[0-9]+]],
// A slot-kind offset inside the block, for contrast with the space kind below.
// CHECK: = offset(9 : Int, {{[0-9]+}} : Int)
// CHECK: {{^}}let %[[CONTAINER]]{{[^0-9]}}{{.*}}= varLayout(%{{[0-9]+}},
// CHECK: = offset(12 : Int, 0 : Int)

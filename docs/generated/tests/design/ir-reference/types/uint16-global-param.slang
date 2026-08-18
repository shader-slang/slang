//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=b3e088ac3dc9706ad8749cb8f9a4f8033e0bd510147eb7d3d1d0a3563b2dd6ca
//META: purpose=Expansion: a scalar UInt16 surfaces as the IR type of a uniform uint16_t global_param.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a scalar UInt16 surfaces as the IR type of a uniform uint16_t
// global_param. A bare `uniform uint16_t` exercises the scalar storage
// form for the narrow unsigned type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint16_t v;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(v);
}

// IR: let {{.*}}%v{{.*}}: UInt16{{.*}}= global_param

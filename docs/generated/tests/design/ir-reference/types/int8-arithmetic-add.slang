//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=b3e088ac3dc9706ad8749cb8f9a4f8033e0bd510147eb7d3d1d0a3563b2dd6ca
//META: purpose=Expansion: arithmetic add on Int8 operands produces an Int8-typed IR value, not auto-widened.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// arithmetic add on Int8 operands produces an Int8-typed IR value, not
// auto-widened. Adding two `int8_t` globals yields an `add`
// instruction whose result type stays `Int8`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t a;
uniform int8_t b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int8_t s = int8_t(a + b);
    buf[0] = int(s);
}

// IR: {{.*}}: Int8{{.*}}= add(%a, %b)

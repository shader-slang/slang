//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=b3e088ac3dc9706ad8749cb8f9a4f8033e0bd510147eb7d3d1d0a3563b2dd6ca
//META: purpose=Expansion: widening conversion from Half to Float yields a Float-typed floatCast value, complementing the narrow-to-wide direction.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// widening conversion from Half to Float yields a Float-typed
// floatCast value, complementing the narrow-to-wide direction. A
// `float(v)` widening from a `half` value produces a `floatCast`
// instruction whose result is `Float`, pulling from the narrowest to
// the platform-default float.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    float widened = float(v);
    buf[0] = widened;
}

// IR: let {{.*}}%widened{{.*}}: Float{{.*}}= floatCast(%v)

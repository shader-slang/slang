//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=Expansion: widening conversion from Half to Float yields a Float-typed floatCast value, complementing the narrow-to-wide direction.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `Half` and `Float` as distinct opcodes. A
// `float(v)` widening from a `half` value produces a `floatCast`
// instruction whose result is `Float`, pulling from the narrowest
// to the platform-default float.

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

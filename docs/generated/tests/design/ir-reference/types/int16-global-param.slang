//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=90fd91b1a21860b3569e7b02fb36035ba69014ded4abafe7604e89fcb5aea51a
//META: purpose=Expansion: a scalar Int16 surfaces as the IR type of a uniform int16_t global_param.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Basic scalar types table lists `Int16` as the 16-bit
// signed integer. A bare `uniform int16_t` exercises the scalar
// storage form for the narrow signed type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int16_t v;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(v);
}

// IR: let {{.*}}%v{{.*}}: Int16{{.*}}= global_param

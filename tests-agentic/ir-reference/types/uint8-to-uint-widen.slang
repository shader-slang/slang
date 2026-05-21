//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=Expansion: widening conversion from UInt8 to UInt yields a UInt-typed intCast value.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc separates `UInt8` from the wider `UInt`. A `uint(v)`
// widening from a `uint8_t` value produces an `intCast` whose
// result type is `UInt`, complementing the narrow direction
// exercised by other expansion tests.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint8_t v;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    uint widened = uint(v);
    buf[0] = widened;
}

// IR: let {{.*}}%widened{{.*}}: UInt{{.*}}= intCast(%v)

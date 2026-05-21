//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=Expansion: narrowing from UInt64 to UInt16 yields a UInt16-typed cast value, distinguishing the two unsigned widths.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `UInt64` (largest) and `UInt16` (narrow) as
// distinct opcodes. A `uint16_t(v)` cast from a `uint64_t` value
// produces a `Cast` instruction whose result type is `UInt16`,
// pulling the value down from the largest unsigned scalar.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint64_t v;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    uint16_t narrowed = uint16_t(v);
    buf[0] = uint(narrowed);
}

// IR: let {{.*}}%narrowed{{.*}}: UInt16{{.*}}= {{.*}}Cast

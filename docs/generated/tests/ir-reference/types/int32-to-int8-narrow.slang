//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=Expansion: narrowing conversion from Int to Int8 yields a value typed Int8 in the dump.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc distinguishes `Int` (32-bit) and `Int8` (8-bit) as
// separate scalar opcodes. An `int8_t(v)` narrowing produces a
// value of IR type `Int8`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int v;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int8_t narrowed = int8_t(v);
    buf[0] = int(narrowed);
}

// IR: let {{.*}}%narrowed{{.*}}: Int8{{.*}}= {{.*}}Cast

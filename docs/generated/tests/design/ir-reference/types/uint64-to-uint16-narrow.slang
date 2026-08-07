//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=221e203d328edd373a1da34064baee6a2aab4c19c403c7438de72b93ad6ee5f5
//META: purpose=Expansion: narrowing from UInt64 to UInt16 yields a UInt16-typed cast value, distinguishing the two unsigned widths.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// narrowing from UInt64 to UInt16 yields a UInt16-typed cast value,
// distinguishing the two unsigned widths. A `uint16_t(v)` cast from a
// `uint64_t` value produces a `Cast` instruction whose result type is
// `UInt16`, pulling the value down from the largest unsigned scalar.

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

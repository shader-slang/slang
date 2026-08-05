//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=221e203d328edd373a1da34064baee6a2aab4c19c403c7438de72b93ad6ee5f5
//META: purpose=Expansion: narrowing conversion from Int to Int8 yields a value typed Int8 in the dump.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// narrowing conversion from Int to Int8 yields a value typed Int8 in
// the dump. An `int8_t(v)` narrowing produces a value of IR type
// `Int8`.

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

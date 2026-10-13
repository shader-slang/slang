//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies `Optional<float>` in a constant buffer is laid out as a (value, bool) tuple with the value first at offset 0 and the flag at offset 4.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_createTypeLayout`'s `OptionalType` arm rewrites `Optional<T>` into a
// two-element tuple and lays that out as a struct. The source comment notes the
// chosen `(T, bool)` field order differs from the natural-layout
// implementation's `(bool, T)`, so the order is worth pinning: `Offset 0` for
// the value and `Offset 4` for the flag is the tuple ordering, and the reverse
// would be the natural-layout ordering. The enclosing struct's `tail` field
// lands at 16 because std140 rounds the nested struct up to 16-byte alignment.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

struct S { Optional<float> maybe; float tail; }
ConstantBuffer<S> cb;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.maybe.hasValue ? cb.maybe.value : cb.tail;
}

//SPV-DAG: OpMemberDecorate %_slang_Optional_float_std140 0 Offset 0
//SPV-DAG: OpMemberDecorate %_slang_Optional_float_std140 1 Offset 4
//SPV-DAG: OpMemberDecorate %S_std140 0 Offset 0
//SPV-DAG: OpMemberDecorate %S_std140 1 Offset 16

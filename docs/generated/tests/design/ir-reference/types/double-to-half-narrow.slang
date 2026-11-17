//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=fc3f64cf1767fc468943c658dcb39e5c71cdbb3af8d9335eb8de55db2a3c15f7
//META: purpose=Expansion: narrowing conversion from Double to Half yields a Half-typed floatCast value.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// narrowing conversion from Double to Half yields a Half-typed
// floatCast value. A `half(v)` narrowing from a `double` value
// produces a `floatCast` instruction whose result type is `Half`,
// pulling from the widest to the narrowest float.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform double v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    half h = half(v);
    buf[0] = float(h);
}

// IR: let {{.*}}%h{{.*}}: Half{{.*}}= floatCast(%v)

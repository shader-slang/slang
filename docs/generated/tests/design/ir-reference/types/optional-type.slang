//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vectors-matrices-and-composite
//META: doc_section_digest=f6cc1906090c3a2ff4445cfbcbe2318b867549d8261d32762ca2f8abf427d1bf
//META: purpose=The Optional(valueType) IR type is produced for an Optional<T> return value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Optional(valueType) IR type is produced for an Optional<T>
// return value. A helper returning `Optional<int>` surfaces
// `Optional(Int)` as its Func result type in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

Optional<int> tryPositive(int x) { if (x > 0) return x; return none; }

[numthreads(1, 1, 1)]
void main()
{
    Optional<int> o = tryPositive(a);
    buf[0] = o.value;
}

// IR-LABEL: func %tryPositive
// IR-SAME: Func(Optional(Int), Int)

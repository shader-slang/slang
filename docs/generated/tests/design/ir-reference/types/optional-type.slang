//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vectors-matrices-and-composite
//META: doc_section_digest=54f732694185a0ac0e3a99ea776a2df81f46ad04014e1f6987caf714e9e03dcc
//META: purpose=The Optional(valueType) IR type is produced for an Optional<T> return value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Optional(valueType) IR type is produced for an Optional<T>
// return value. A helper returning `Optional<int>` surfaces
// `Optional(Int)` as its Func result type in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

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

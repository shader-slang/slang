//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#vectors-matrices-and-composite
//META: doc_section_digest=a2958c27179f43de9a6c91173654a84b9300b21241a75525e0ed05ed4d9bc705
//META: purpose=The Optional(valueType) IR type is produced for an Optional<T> return value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Vectors, matrices, and composite" table lists
// `Optional` as a composite type opcode with operand
// `valueType: IRType`. A helper returning `Optional<int>`
// surfaces `Optional(Int)` as its Func result type in the dump.

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

//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#tuples-packs-and-target-tuples
//META: doc_section_digest=0ce529a37c66a012b4e63b3e8fc7709640b11bd360f2057175f20a6473f97c44
//META: purpose=A makeTuple call yields a value of IR type tuple_type(<T1>, <T2>, ...).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Tuples, packs, and target tuples" table lists
// `tuple_type` with operands `types: IRType...` as the IR encoding
// of a heterogeneous tuple. `makeTuple(a, b)` for a Float and an
// Int surfaces `tuple_type(Float, Int)` as the value's type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform int b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    var t = makeTuple(a, b);
    buf[0] = t._0 + float(t._1);
}

// IR-LABEL: func %main
// IR: tuple_type(Float, Int){{.*}}= makeTuple(

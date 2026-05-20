//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#tuples-packs-and-target-tuples
//META: doc_section_digest=2647b96dc2fd33337f33685e52a4373db41982afb91f8e212a9778646ae44a78
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

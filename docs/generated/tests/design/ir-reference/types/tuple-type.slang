//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#tuples-packs-and-target-tuples
//META: doc_section_digest=5dd886c6a7091235bb66306bec8f28f82567d9db3e4761919dd9f9bc5d6b0b8c
//META: purpose=A makeTuple call yields a value of IR type tuple_type(<T1>, <T2>, ...).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A makeTuple call yields a value of IR type tuple_type(<T1>, <T2>,
// ...). `makeTuple(a, b)` for a Float and an Int surfaces
// `tuple_type(Float, Int)` as the value's type.

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

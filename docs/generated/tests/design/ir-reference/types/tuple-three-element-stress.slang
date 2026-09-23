//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#tuples-packs-and-target-tuples
//META: doc_section_digest=340a6086a58d3b4b09f103090fa05f6f3ae19060b59ae154ea1dba8c258d6a25
//META: purpose=Stress: a three-element heterogeneous tuple surfaces tuple_type(Float, Int, Bool) with all operands.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a three-element heterogeneous tuple surfaces tuple_type(Float, Int,
// Bool) with all operands. A three-element tuple verifies that the
// variadic operand list preserves all element types in order —
// distinct from the two-element smoke test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform float a;
uniform int b;
uniform bool c;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    var t = makeTuple(a, b, c);
    buf[0] = t._0 + float(t._1) + (t._2 ? 1.0 : 0.0);
}

// IR-LABEL: func %main
// IR: tuple_type(Float, Int, Bool){{.*}}= makeTuple(

//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#tuples-packs-and-target-tuples
//META: doc_section_digest=0ce529a37c66a012b4e63b3e8fc7709640b11bd360f2057175f20a6473f97c44
//META: purpose=Stress: a three-element heterogeneous tuple surfaces tuple_type(Float, Int, Bool) with all operands.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `tuple_type(types...)` is variadic. A three-element
// tuple verifies that the variadic operand list preserves all
// element types in order — distinct from the two-element smoke
// test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

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

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A `Tuple<int8_t, float, bool>` constructor lowers to a `makeTuple(...)` whose result IR type is `tuple_type(Int8, Float, Bool)` carrying each element's lowered IR type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: heterogeneous tuple over mixed precisions. The doc
// does not explicitly list `Tuple`, but tuples are observable in
// IR as `tuple_type(...)` with one operand per element type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t a;
uniform float b;
uniform bool c;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Tuple<int8_t, float, bool> t = makeTuple(a, b, c);
    buf[0] = int(t._0) + int(t._1) + (t._2 ? 1 : 0);
}

// CHECK-LABEL: func %main
// CHECK: tuple_type(Int8, Float, Bool){{.*}}= makeTuple

//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T20:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
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

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A `Tuple<half, uint16_t>` lowers to `tuple_type(Half, UInt16)` — narrow-precision pair carries through tuple IR type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: heterogeneous tuple over two narrow types.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half a;
uniform uint16_t b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Tuple<half, uint16_t> t = makeTuple(a, b);
    buf[0] = float(t._0) + float(t._1);
}

// CHECK-LABEL: func %main
// CHECK: tuple_type(Half, UInt16){{.*}}= makeTuple

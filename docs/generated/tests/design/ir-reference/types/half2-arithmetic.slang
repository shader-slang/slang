//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=fede71bd73c83957c94e27eab0113790d3ae8864e230db8dde234dca52b8d824
//META: purpose=Expansion: vector add on Vec(Half, 2 : Int) operands yields an add whose result type stays Vec(Half, 2 : Int).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// vector add on Vec(Half, 2 : Int) operands yields an add whose result
// type stays Vec(Half, 2 : Int). Adding two `half2` globals produces a
// vector `add` whose result is a `Vec(Half, 2 : Int)` value —
// arithmetic on the smallest multi-element half vector preserves both
// width and shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half2 a;
uniform half2 b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    half2 c = a + b;
    buf[0] = float(c.x) + float(c.y);
}

// IR: {{.*}}: Vec(Half, 2 : Int){{.*}}= add(%a, %b)

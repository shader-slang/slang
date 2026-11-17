//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=bb574530be2949c3439b7d94ef06cf75ea770a6c6d65a1d5d5ef56c055963e3b
//META: purpose=Expansion: vector add on Vec(Half, 3 : Int) operands produces an add whose result type stays Vec(Half, 3 : Int).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// vector add on Vec(Half, 3 : Int) operands produces an add whose
// result type stays Vec(Half, 3 : Int). Adding two `half3` globals
// produces a vector `add` whose result is itself a `Vec(Half, 3 :
// Int)` value — arithmetic on the narrow-element vector preserves both
// element type and width.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform half3 a;
uniform half3 b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    half3 c = a + b;
    buf[0] = float(c.x) + float(c.y) + float(c.z);
}

// IR: {{.*}}: Vec(Half, 3 : Int){{.*}}= add(%a, %b)

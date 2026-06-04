//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=4430d3efb9516cd383356961351a223b29ad1c948e7c5ea9bf0a36d832a4b940
//META: purpose=A function parameter declared with the precise modifier carries a precise decoration with no operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc records precise ("Requests bit-precise math") with no
// operands and AST origin "precise modifier".

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

float bumpy(precise float x) { return x * x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = bumpy(float(tid.x));
}

// CHECK: [precise]

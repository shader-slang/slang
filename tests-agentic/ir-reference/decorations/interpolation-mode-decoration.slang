//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=4430d3efb9516cd383356961351a223b29ad1c948e7c5ea9bf0a36d832a4b940
//META: purpose=A field declared with a nointerpolation modifier produces an interpolationMode decoration with the mode operand recording the chosen mode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc table lists interpolationMode with operand
// modeOperand: IRConstant, AST origin "linear / noperspective /
// etc. modifiers". The exact integer encoding varies, so the test
// pins the decoration shape but wildcards the mode integer.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    float pos : SV_Position;
    nointerpolation float w : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    V v;
    v.pos = 0.0;
    v.w = float(tid.x);
    buf[0] = v.w;
}

// CHECK: [interpolationMode({{[0-9]+}} : Int)]

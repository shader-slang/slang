//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#layout-and-binding
//META: doc_section_digest=53c33072d342ae9430710df507cdde301b5918b46f7119412b00900374c8e222
//META: purpose=A [vk::location(N)] modifier lowers to a glslLocation decoration carrying the integer location operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc records `glslLocation` with operand `location: IRIntLit`
// and AST origin `[vk::location(...)]` modifier. Confirm the
// integer N flows through unchanged.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    float pos : SV_Position;
    [vk::location(7)] linear float u : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    V v;
    v.pos = 0.0;
    v.u = float(tid.x);
    buf[0] = v.u;
}

// CHECK: [glslLocation(7 : Int)]

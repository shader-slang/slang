//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=A `float4(a, b, c, d)` constructor lowers to the documented "aggregate constructors" value family — the `makeVector` opcode collecting per-element operands into a `Vec(Float, 4)` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's value-instructions row names "aggregate constructors" as
// one of the value-family groupings. The user surface for a vector
// constructor is `float4(...)`, and the corresponding IR opcode is
// `makeVector` (with C++ wrapper `IRMakeVector`) — its variadic
// element operands match the documented "collects per-element
// operands" shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform float b;
uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = float4(a, b, 0, 1);
}

// IR-LABEL: func %main
// IR: makeVector(%a, %b, 0 : Float, 1 : Float)

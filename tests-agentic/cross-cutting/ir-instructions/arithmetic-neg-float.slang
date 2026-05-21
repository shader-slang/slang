//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Boundary probe of the documented arithmetic family on a float operand: `-a` lowers to the unary `neg` opcode (sibling of the binary `add`/`sub`/`mul`/`div` set the doc names).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The existing `arithmetic-neg-int.slang` probes `neg` on an integer
// operand; this probe exercises the same opcode on the float side,
// pinning down the documented arithmetic family's behavior across
// the integer/float boundary axis.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = -a;
}

// IR-LABEL: func %main
// IR: neg(%a)

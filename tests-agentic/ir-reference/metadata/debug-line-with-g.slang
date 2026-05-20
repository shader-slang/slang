//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/metadata.md#debugline
//META: doc_section_digest=7e4bc2b57f9be05553cc5289795959ecaec4af0d5a1532efa5ba66d1ab6e08f1
//META: purpose=DebugLine pins an instruction to a (file, startLine, startCol, endLine, endCol) range and is emitted in source order under -g.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Notable-opcodes "DebugLine" section: `DebugLine` pins an
// instruction to a (file, startLine, startCol, endLine, endCol)
// range and is *not* attached as a decoration so that the location
// flows with the instruction across CFG transformations.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: DebugLine(%{{[0-9]+}}, {{[0-9]+}} : UInt, {{[0-9]+}} : UInt, {{[0-9]+}} : UInt, {{[0-9]+}} : UInt)

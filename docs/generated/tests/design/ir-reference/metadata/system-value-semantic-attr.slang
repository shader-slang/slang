//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#usersemantic-vs-systemvaluesemantic
//META: doc_section_digest=0bd97c5a925ac4c580456f31d346716b390324530e12eae524e3ab758dc8f187
//META: purpose=The systemValueSemantic Attr opcode lowers a parameter with an SV_* semantic and tags it inside the layout tree.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Notable-opcodes "userSemantic vs systemValueSemantic"
// section: an `SV_*` system value lowers to a `systemValueSemantic`
// Attr opcode whose first operand is the semantic name string and
// second operand is the index integer.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = systemValueSemantic("SV_DispatchThreadID", {{[0-9]+}} : Int)

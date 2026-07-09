//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=5749d9886287d8389af87cb5a6a536a28da48a421ac9a27f86809fca6a7b5676
//META: purpose=Builtin bitwise and shift operators over builtin scalar operands are rewritten by the checker and evaluate correctly through IR lowering.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the checker's builtin-operator rewrite covers the bitwise
// and shift operator classes on scalar operands. Operands pass through
// a function so IR lowering (a documented consumer of the resolved
// operation kind) has to lower real runtime ops; CHECK asserts each
// operator's arithmetic identity.

//TEST:INTERPRET(filecheck=CHECK):

uint pass(uint v)
{
    return v;
}

void main()
{
    uint a = pass(0xF0u);
    uint b = pass(0x3Cu);
    // 0xF0 & 0x3C = 0x30 (48); 0xF0 | 0x3C = 0xFC (252); 0xF0 ^ 0x3C = 0xCC (204)
    // CHECK: and=48 or=252 xor=204
    printf("and=%u or=%u xor=%u\n", a & b, a | b, a ^ b);
    // 0x3C << 2 = 0xF0 (240); 0xF0 >> 4 = 0x0F (15)
    // CHECK: shl=240 shr=15
    printf("shl=%u shr=%u\n", b << 2, a >> 4);
}

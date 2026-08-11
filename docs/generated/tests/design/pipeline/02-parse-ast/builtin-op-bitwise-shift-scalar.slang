//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=bf6116c2e5b561cd38063b60dd513143618c8f41f5c801a0b352865012956ca2
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

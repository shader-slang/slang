//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#enumdecl-and-enumcasedecl
//META: doc_section_digest=b72726890459268d21bfd18ef8977151c7699c0ba49e17682e79c4cb2cfa2ff4
//META: purpose=An EnumCaseDecl carries the case's tag value; cases compare equal to the integer they were declared with.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of EnumCaseDecl. The enum case stores
// its tag value (the `= 1`, `= 2`, ... portion). Converting the case
// to its underlying integer must yield that exact tag value.

//TEST:INTERPRET(filecheck=CHECK):

enum E { A = 1, B = 5, C = 9 }

void main()
{
    //CHECK: 1 5 9
    printf("%d %d %d\n", int(E::A), int(E::B), int(E::C));
}

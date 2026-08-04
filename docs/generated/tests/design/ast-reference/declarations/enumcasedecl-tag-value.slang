//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#enumdecl-and-enumcasedecl
//META: doc_section_digest=c018493cff4ea0d125fd96032328c7a624df200e9019bef0883c86de143f6642
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

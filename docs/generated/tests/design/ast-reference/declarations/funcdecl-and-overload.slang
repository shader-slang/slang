//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=A FuncDecl is an ordinary function declaration; two FuncDecls sharing a name but differing in parameter type overload independently.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that two same-named function declarations coexist as separate
// declarations and that the call site picks one by argument type. Each
// overload returns a distinct marker value; the CHECK asserts both markers,
// so a collapse into a single declaration would change the output.

//TEST:INTERPRET(filecheck=CHECK):

int classify(int x)   { return 1; }
int classify(float x) { return 2; }

void main()
{
    //CHECK: 1 2
    printf("%d %d\n", classify(3), classify(2.5f));
}

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=A FuncDecl declared inside an aggregate becomes a method whose body can read the receiver's fields.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a function declared as a member of a struct is callable
// through a receiver and that unqualified field names inside its body refer
// to that receiver. The CHECK asserts a value derived from the receiver's
// field, which requires the implicit `this` binding.

//TEST:INTERPRET(filecheck=CHECK):

struct V
{
    int x;
    int doubled() { return x * 2; }
}

void main()
{
    V v;
    v.x = 21;
    //CHECK: 42
    printf("%d\n", v.doubled());
}

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_section_digest=1d025242d1797d4e136aa1be3b95725504ad5b96bc6526725ad99442cfbb5be0
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-packbranchtypeexpr
//META: purpose=The pointer type spelling `T*` is accepted in a parameter's type position and resolves to a pointer to `T`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A type spelling like `S*` goes through the same expression parser as a value
// expression and only becomes a type during checking, so the test's job is to
// show the spelling is accepted in a type position and produces the right
// type. A function declares an `S*` parameter and dereferences it; the CHECK
// asserts the callee read the caller's struct through that parameter.

//TEST:INTERPRET(filecheck=CHECK):

struct S { int v; }

int readIt(S* p) { return p->v; }

void main()
{
    S s = { 21 };
    // CHECK: v=21
    printf("v=%d\n", readIt(__getAddress(s)));
}
